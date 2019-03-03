//
//  TFT_eTouchBase.cpp
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

#ifdef ESP_PLATFORM
#include <FS.h>
#include <SPIFFS.h>
#endif

#include <TFT_eTouchBase.h>

TFT_eTouchBase::TFT_eTouchBase(uint8_t cs_pin, uint8_t penirq_pin, SPIClass& spi)
: spi_(spi)
, cs_(cs_pin)
, penirq_(penirq_pin)
//, raw_.x(0), raw_.y(0)
//, raw_z1_(0), raw_z2_(0)
//, rz_(0xffff)
#ifdef TOUCH_USE_PENIRQ_CODE
, update_allowed_(true)
#endif // end TOUCH_USE_PENIRQ_CODE
, drop_first_measures_(0)
, z_once_measure_(false)
, z_first_measure_(true)
, z_local_min_measure_(false)
, count_measure_(3)
#ifdef TOUCH_USE_AVERAGING_CODE
, averaging_measure_(false)
, ignore_min_max_measure_(false)
#endif // end TOUCH_USE_AVERAGING_CODE

, raw_valid_min_(25), raw_valid_max_(4000)
, last_measure_time_ms_(0), measure_wait_ms_(5)
, rx_plate_(1000/3)
, rz_threshold_(1000)
#ifdef TOUCH_USE_USER_CALIBRATION
, acurate_difference_(10)
#endif // TOUCH_USE_USER_CALIBRATION
{
  calibation_ = { 300, 3700, 300, 3700, 2 };
}

void TFT_eTouchBase::init(bool spi_init)
{
  if (spi_init) spi_.begin();
  pinMode(cs_, OUTPUT);
  digitalWrite(cs_, HIGH);

#ifdef ESP_PLATFORM
  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formating SPIFFS file system");

    SPIFFS.format();
    SPIFFS.begin();
  }
#endif
}


bool TFT_eTouchBase::getRaw(uint16_t& x, uint16_t& y, uint16_t& z1, uint16_t& z2, uint16_t& rz)
{
  update(false);
  if (is_touched()) {
    x = raw_.x;
    y = raw_.y;
    z1 = raw_.z1;
    z2 = raw_.z2;
    rz = raw_.rz;
    return true;
  }
  return false;
}

bool TFT_eTouchBase::getRaw(Measure& raw)
{
  update(false);
  if (is_touched()) {
    raw = raw_;
    return true;
  }
  return false;
}

void TFT_eTouchBase::waitPenUp()
{
  if (!is_touched()) return;

  while (raw_.z1 != 0) {
    delay(measure_wait_ms_ + 1);
    update(true);
  }
}


bool TFT_eTouchBase::readCalibration(const char* descr)
{
  bool ret = false;
#ifdef ESP_PLATFORM
  if (SPIFFS.exists(descr)) {
    File calfile = SPIFFS.open(descr, "r");
    if (calfile) {
      Calibation data;
      if (calfile.readBytes((char *)&data, sizeof(data)) == sizeof(data)) {
        ret = true;
        calibation_ = data;
        Serial.printf("Calibration: %s x %u, %u, y %u, %u, r %u\n", descr, calibation_.x0, calibation_.x1, calibation_.y0, calibation_.y1, calibation_.rel_rotation);
      }
      else Serial.println("Calibration File read error");
      calfile.close();
    }
    else Serial.println("Calibration File open for read error");
  }
#else
  Serial.println("TFT_eTouchBase::readCalibration() not implemented");
#endif
  return ret;
}


bool TFT_eTouchBase::writeCalibration(const char* descr)
{
  bool ret = false;
#ifdef ESP_PLATFORM
  File calfile = SPIFFS.open(descr, "w");
  if (calfile) {
    calfile.write((const unsigned char *)&calibation_, sizeof(calibation_));
    calfile.close();
    ret = true;
  }
  else Serial.println("Calibration File open for write error");
#else
  Serial.println("TFT_eTouchBase::writeCalibration() not implemented");
#endif
  return ret;
}


void TFT_eTouchBase::update(bool only_z1)
{
#ifdef TOUCH_USE_PENIRQ_CODE
	if (!update_allowed_) return;
#else
  // cant query PENIRQ for LOW, but when all measures in range then dispay is touched
#endif // end TOUCH_USE_PENIRQ_CODE

	uint32_t now = micros();
	if (now < last_measure_time_ms_ + measure_wait_ms_ * 1000) return;
  last_measure_time_ms_ = now;

  fetch_raw(only_z1);

#ifdef TOUCH_SERIAL_CONVERSATION_TIME
  Serial.printf("TFT_eTouchBase::update() %d microseconds\n", micros() - last_measure_time_ms_);
#endif
  
#ifdef TOUCH_USE_PENIRQ_CODE
  if (penirq_ != 0xff) {
    if (only_z1) update_allowed_ = raw_.z1 > 0;
	  else update_allowed_ = valid();
  }
#endif // end TOUCH_USE_PENIRQ_CODE
}


// Differential Measure (SER/DFR low)
#define X_MEASURE_DFR   0b11010001 // 0b11010011 also works
#define Y_MEASURE_DFR   0b10010001 // 0b10010011
#define Z1_MEASURE_DFR  0b10110001
#define Z2_MEASURE_DFR  0b11000001

#define OFF_MEASURE     0b10010000

// Single Ended Measure (SER/DFR high)
#define X_MEASURE_SER   0b11010100
#define Y_MEASURE_SER   0b10010100
#define Z1_MEASURE_SER  0b10110101
#define Z2_MEASURE_SER  0b11000101

#ifdef TOUCH_USE_DIFFERENTIAL_MEASURE
#define X_MEASURE   X_MEASURE_DFR
#define Y_MEASURE   Y_MEASURE_DFR
#define Z1_MEASURE  Z1_MEASURE_DFR
#define Z2_MEASURE  Z2_MEASURE_DFR
#else
#define X_MEASURE   X_MEASURE_SER
#define Y_MEASURE   Y_MEASURE_SER
#define Z1_MEASURE  Z1_MEASURE_SER
#define Z2_MEASURE  Z2_MEASURE_SER
#endif

void TFT_eTouchBase::fetch_raw(bool only_z1)
{
  bool has_touch = true;
  uint16_t data1, data2;
  uint8_t ctrl = X_MEASURE; // X-POSITION Measure
  uint8_t drop_cnt = drop_first_measures_;
  
  if (z_first_measure_ && !z_once_measure_) {
    ctrl = Z1_MEASURE; // Z1-POSITION Measure
  }
  spi_start();
  
  if (z_once_measure_) {
    z_first_measure_ = true; // then we do it first
    spi_.transfer(Z1_MEASURE); // Z1 Measure
    while (has_touch && drop_cnt-- > 0) {
      data2 = (spi_.transfer16(Z1_MEASURE) >> 3) & 0x0fff;
      if (!in_range(data2)) {
        has_touch = false;
        raw_.z1 = 0;
      }
    }
    drop_cnt = drop_first_measures_;
    if (has_touch) {
      if (z_local_min_measure_) { // read z1 until grows
        data2 = 0;
        do {
          data1 = data2;
          data2 = (spi_.transfer16(Z1_MEASURE) >> 3) & 0x0fff;
        } while (data1 < data2);
      }
      if (only_z1) raw_.z1 = (spi_.transfer16(OFF_MEASURE) >> 3) & 0x0fff;
      else         raw_.z1 = (spi_.transfer16(Z2_MEASURE) >> 3) & 0x0fff; // Z2 Measure
      if (!in_range(raw_.z1)) {
        has_touch = false;
        raw_.z1 = 0;
      }
      else {
        if (only_z1) {
          spi_end();
          return;
        }
        while (drop_cnt-- > 0) spi_.transfer16(Z2_MEASURE);
        drop_cnt = drop_first_measures_;
//        if (drop_first_measure_) spi_.transfer16(Z2_MEASURE);
        raw_.z2 = (spi_.transfer16(ctrl) >> 3) & 0x0fff; // X Measure
        if (!in_range(raw_.z2)) {
          has_touch = false;
        }
      }
    }
  }
  else {
    if (only_z1) ctrl = Z1_MEASURE;
    spi_.transfer(ctrl); // X or Z1 Measure
    raw_.z1 = 0;
  }
  
#ifdef TOUCH_USE_AVERAGING_CODE
  if (!averaging_measure_) {
    ignore_min_max_measure_ = false;
  }
#endif // end TOUCH_USE_AVERAGING_CODE

  while (has_touch) {
    if (count_measure_ == 0) {
      // Figure 10
      data1 = 0xffff;
      do {
        data2 = data1;
        data1 = (spi_.transfer16(ctrl) >> 3) & 0x0fff; // X, Y, Z1 or Z2 Measure
      } while (in_range(data1) && data1 != data2);  // wait until stable
    }
    else {
      // Figure 11 or averaging
#ifdef TOUCH_USE_AVERAGING_CODE
      uint16_t min = 0xffff, max = 0;
      data2 = count_measure_;
      if (averaging_measure_) {
        while (has_touch && drop_cnt-- > 0) {
          data1 = (spi_.transfer16(ctrl) >> 3) & 0x0fff;
          if (!in_range(data1)) {
            has_touch = false;
          }
        }
        drop_cnt = drop_first_measures_;
        if (!has_touch) break;
//        while (drop_cnt-- > 0) spi_.transfer16(ctrl);
//        if (drop_first_measure_) spi_.transfer16(ctrl);
        if (ignore_min_max_measure_) {
          data2 += 2;
        }
        if (data2 > 16) { // 16 is max! (data1 is 16bit one sum value 12bit, 4bit's for 16 Values)
          data2 = 16;
          if (ignore_min_max_measure_) count_measure_ = 14;
          else                         count_measure_ = 16;
        }
      }
#else
      data2 = count_measure_;
#endif // end TOUCH_USE_AVERAGING_CODE
      data1 = 0;
      uint8_t next_ctrl = ctrl;
      while (data2--) {
        if (data2 == 0) { // last measure of axis, switch ctrl to next
          if (ctrl == X_MEASURE) {
            next_ctrl = Y_MEASURE; // Y-POSITION Measure
          }
          else if (ctrl == Y_MEASURE) {
            if (z_first_measure_) next_ctrl = OFF_MEASURE;
            else next_ctrl = Z1_MEASURE; // Z1-POSITION Measure
          }
          else if (ctrl == Z1_MEASURE) {
            if (!z_local_min_measure_) { // then later
              next_ctrl = Z2_MEASURE; // Z2-POSITION Measure
            }
            else if (only_z1) ctrl = OFF_MEASURE;
          }
          else if (ctrl == Z2_MEASURE) { // Z2 Measure done
            if (!z_first_measure_) next_ctrl = OFF_MEASURE;
            else next_ctrl = X_MEASURE; // X-POSITION Measure
          }
#ifdef TOUCH_USE_AVERAGING_CODE
          if (!averaging_measure_) {
            data1 = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff; // take n'th measure of X, Y, Z1 or Z2
          }
#else
          data1 = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff; // take n'th measure of X, Y, Z1 or Z2
#endif // end TOUCH_USE_AVERAGING_CODE
        }
#ifdef TOUCH_USE_AVERAGING_CODE
        if (averaging_measure_) {
          uint16_t data = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff; // X, Y, Z1 or Z2 Measure
          data1 += data;
          if (ignore_min_max_measure_) {
            if (min > data) min = data;
            if (max < data) max = data;
          }
        }
        else {
          // dummy read when not n'th measure
          if (data2 > 0) {
//            spi_.transfer16(next_ctrl);
            data1 = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff;
            if (!in_range(data1)) {
              data2 = 0;
//              has_touch = false;
            }
          }
        }
#else
        else {
          // dummy read when not n'th measure
//          spi_.transfer16(next_ctrl);
          data1 = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff;
          if (!in_range(data1)) {
            data2 = 0;
          }
        }
#endif // end TOUCH_USE_AVERAGING_CODE
      }
#ifdef TOUCH_USE_AVERAGING_CODE
      if (averaging_measure_) {
        if (ignore_min_max_measure_) {
          data1 -= (min + max);
        }
        data1 /= count_measure_;
      }
#endif // end TOUCH_USE_AVERAGING_CODE
    }

    if (!in_range(data1)) {
      has_touch = false;
    }
    else if (ctrl == X_MEASURE) { // X Measure done
      raw_.x = data1;
      ctrl = Y_MEASURE; // Y-POSITION Measure
    }
    else if (ctrl == Y_MEASURE) { // Y Measure done
      raw_.y = data1;
      if (z_first_measure_) break;
      ctrl = Z1_MEASURE; // Z1-POSITION Measure
    }
    else if (ctrl == Z1_MEASURE) { // Z1 Measure done
      if (z_local_min_measure_) { // read z1 until grows
        if (raw_.z1 >= data1) {
          ctrl = Z2_MEASURE; // Z2-POSITION Measure
          if (count_measure_ > 0) {
            if (!only_z1) spi_.transfer16(ctrl); // dummy read of last z1, because next transfer16() must return z2
          }
        }
      }
      else {
        ctrl = Z2_MEASURE; // Z2-POSITION Measure
      }
      raw_.z1 = data1;
      if (only_z1 && ctrl == Z2_MEASURE) {
        if (count_measure_ == 0) {
          spi_.transfer16(OFF_MEASURE); // set power down mode
        }
        spi_end();
        return;
      }
    }
    else if (ctrl == Z2_MEASURE) { // Z2 Measure done
      raw_.z2 = data1;
      if (!z_first_measure_) break;
      ctrl = X_MEASURE; // X-POSITION Measure
    }
  }
  if (count_measure_ == 0 || !has_touch) {
    spi_.transfer16(OFF_MEASURE); // set power down mode
  }
  spi_end();

#ifdef TOUCH_SERIAL_DEBUG_FETCH //
  if (!has_touch) {
    Serial.print("raw measure out of range value: "); Serial.print(data1); Serial.print(" ctrl: 0x"); Serial.println(ctrl, BIN);
  }
#endif

  if (has_touch && raw_.z1 > 0) { // if z1 is 0 we get a division by 0 exception!
    if (raw_.z1 >= raw_.z2) raw_.rz = 0;  // more then 2 Finger
//    else raw_.rz = (uint16_t)((((int32_t)rx_plate_ * raw_.z2 / raw_.z1) * raw_.x / 4096) - (int32_t)rx_plate_ * raw_.x / 4096);
    else raw_.rz = (uint16_t)(((((int32_t)rx_plate_ * raw_.z2 / raw_.z1) * raw_.x) - (int32_t)rx_plate_ * raw_.x) / 4096);
    // Formula from ADS7846 pdf: R_TOUCH = Rx-plate * X-Position/4096 * (Z2/Z1 - 1) ; bud work only with float bud i prefer speed
//    else raw_.rz = (uint16_t)((float)rx_plate_ * raw_.x / 4096.0 * ((float)raw_.z2 / raw_.z1 - 1.0));

//    Serial.print("rz: "); Serial.print((uint16_t)((((int32_t)rx_plate_ * raw_.z2 / raw_.z1) * raw_.x / 4096) - (int32_t)rx_plate_ * raw_.x / 4096));
//    Serial.print(", "); Serial.print((uint16_t)(((((int32_t)rx_plate_ * raw_.z2 / raw_.z1) * raw_.x) - (int32_t)rx_plate_ * raw_.x) / 4096));
//    Serial.print(", "); Serial.print((uint16_t)((float)rx_plate_ * raw_.x / 4096.0 * ((float)raw_.z2 / raw_.z1 - 1.0))); Serial.print(" ");
  }
  else raw_.rz = 0xffff;  // indicate 'no touch'
}


// -- 
#ifdef TOUCH_USE_USER_CALIBRATION
void TFT_eTouchBase::CalibrationPoint::print()
{
#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("x,y scr: ");
  Serial.print(scr_x);
  Serial.print(",");
  Serial.print(scr_y);
  Serial.print(" touch: ");
  Serial.print(touch_x);
  Serial.print(",");
  Serial.println(touch_y);
#endif
}
#endif // TOUCH_USE_USER_CALIBRATION
