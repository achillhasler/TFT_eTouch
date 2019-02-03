#include <TFT_eTouchBase.h>

//
//  TFT_eTouchBase.cpp
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at http://www.boost.org/LICENSE_1_0.txt
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

TFT_eTouchBase::TFT_eTouchBase(uint8_t cs_pin, uint8_t penirq_pin, SPIClass& spi)
: spi_(spi)
, cs_(cs_pin)
, penirq_(penirq_pin)
, raw_x_(0), raw_y_(0)
, raw_z1_(0), raw_z2_(0)
, rz_(0xffff)
#ifdef TOUCH_USE_PENIRQ_CODE
, update_allowed_(true)
#endif // end TOUCH_USE_PENIRQ_CODE

, drop_first_measure_(false)
, z_once_measure_(false)
, z_first_measure_(true)
, z_local_min_measure_(false)
, count_measure_(3)
, averaging_measure_(false)
, ignore_min_max_measure_(false)
, single_ended_reference_mode_(false)

, raw_valid_min_(25), raw_valid_max_(4000)
, last_measure_time_ms_(0), measure_wait_ms_(5)
, rx_plate_(1000/3)
, rz_threshold_(1000)
, acurate_difference_(10)
{
  calibation_ = { 300, 3700, 300, 3700, 2 };
}

bool TFT_eTouchBase::getRaw(uint16_t& x, uint16_t& y, uint16_t& z1, uint16_t& z2, uint16_t& rz)
{
  update(false);
  if (is_touched()) {
    x = raw_x_;
    y = raw_y_;
    z1 = raw_z1_;
    z2 = raw_z2_;
    rz = rz_;
    return true;
  }
  return false;
}

void TFT_eTouchBase::waitPenUp()
{
  if (!is_touched()) return;

  while (raw_z1_ != 0) {
    delay(measure_wait_ms_ + 1);
    update(true);
  }
}

void TFT_eTouchBase::update(bool only_z1)
{
#ifdef TOUCH_USE_PENIRQ_CODE
	if (!update_allowed_) return;
#else
  // cant query PENIRQ for LOW, but when all measures in range then dispay is touched
#endif // end TOUCH_USE_PENIRQ_CODE
  
	uint32_t now = millis();
	if (now < last_measure_time_ms_ + measure_wait_ms_) return;
  last_measure_time_ms_ = now;

  fetch_raw(only_z1);

#ifdef TOUCH_USE_PENIRQ_CODE
  if (penirq_ != 0xff) {
    if (only_z1) update_allowed_ = raw_z1_ > 0;
	  else update_allowed_ = rz_ < rz_threshold_;
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

void TFT_eTouchBase::fetch_raw(bool only_z1)
{
  uint8_t x_ctrl = X_MEASURE_DFR; // Differential Measure (SER/DFR low)
  uint8_t y_ctrl = Y_MEASURE_DFR;
  uint8_t z1_ctrl = Z1_MEASURE_DFR;
  uint8_t z2_ctrl = Z2_MEASURE_DFR;
  if (single_ended_reference_mode_) {
    x_ctrl  = X_MEASURE_SER;  // Single Ended Measure (SER/DFR high)
    y_ctrl  = Y_MEASURE_SER;
    z1_ctrl = Z1_MEASURE_SER;
    z2_ctrl = Z2_MEASURE_SER;
  }

  bool has_touch = true;
  uint16_t data1, data2;
  uint8_t ctrl = x_ctrl; // X-POSITION Measure

  if (z_first_measure_ && !z_once_measure_) {
    ctrl = z1_ctrl; // Z1-POSITION Measure
  }
  spi_start();
  
  if (z_once_measure_) {
    z_first_measure_ = true; // then we do it first
    spi_.transfer(z1_ctrl); // Z1 Measure
    if (drop_first_measure_) spi_.transfer16(z1_ctrl);
    if (z_local_min_measure_) { // read z1 until grows
      data2 = 0;
      do {
        data1 = data2;
        data2 = (spi_.transfer16(z1_ctrl) >> 3) & 0x0fff;
      } while (data1 < data2);
    }
    if (only_z1) raw_z1_ = (spi_.transfer16(OFF_MEASURE) >> 3) & 0x0fff;
    else         raw_z1_ = (spi_.transfer16(z2_ctrl) >> 3) & 0x0fff; // Z2 Measure
    if (!in_range(raw_z1_)) {
      has_touch = false;
      raw_z1_ = 0;
    }
    else {
      if (only_z1) {
        spi_end();
        return;
      }
      if (drop_first_measure_) spi_.transfer16(z2_ctrl);
      raw_z2_ = (spi_.transfer16(ctrl) >> 3) & 0x0fff; // X Measure
      if (!in_range(raw_z2_)) {
        has_touch = false;
      }
    }
  }
  else {
    if (only_z1) ctrl = z1_ctrl;
    spi_.transfer(ctrl); // X or Z1 Measure
    raw_z1_ = 0;
  }
  if (!averaging_measure_) {
    ignore_min_max_measure_ = false;
  }
  while (has_touch) {
    if (count_measure_ == 0) {
      // Figure 10
      data1 = 0xffff;
      do {
        data2 = data1;
        data1 = (spi_.transfer16(ctrl) >> 3) & 0x0fff; // X, Y, Z1 or Z2 Measure
      } while (data1 != data2);  // wait until stable
    }
    else {
      // Figure 11 or averaging
      uint16_t min = 0xffff, max = 0;
      data2 = count_measure_;
      if (averaging_measure_) {
        if (drop_first_measure_) spi_.transfer16(ctrl);
        if (ignore_min_max_measure_) {
          data2 += 2;
        }
        if (data2 > 16) { // 16 is max! (data1 is 16bit one sum value 12bit, 4bit's for 16 Values)
          data2 = 16;
          if (ignore_min_max_measure_) count_measure_ = 14;
          else                         count_measure_ = 16;
        }
      }
      data1 = 0;
      uint8_t next_ctrl = ctrl;
      while (data2--) {
        if (data2 == 0) { // last measure of axis, switch ctrl to next
          if (ctrl == x_ctrl) {
            next_ctrl = y_ctrl; // Y-POSITION Measure
          }
          else if (ctrl == y_ctrl) {
            if (z_first_measure_) next_ctrl = OFF_MEASURE;
            else next_ctrl = z1_ctrl; // Z1-POSITION Measure
          }
          else if (ctrl == z1_ctrl) {
            if (!z_local_min_measure_) { // then later
              next_ctrl = z2_ctrl; // Z2-POSITION Measure
            }
            else if (only_z1) ctrl = OFF_MEASURE;
          }
          else if (ctrl == z2_ctrl) { // Z2 Measure done
            if (!z_first_measure_) next_ctrl = OFF_MEASURE;
            else next_ctrl = x_ctrl; // X-POSITION Measure
          }
          if (!averaging_measure_) {
            data1 = (spi_.transfer16(next_ctrl) >> 3) & 0x0fff; // take n'th measure of X, Y, Z1 or Z2
          }
        }
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
          if (data2 > 0) spi_.transfer16(next_ctrl);
        }
      }
      if (averaging_measure_) {
        if (ignore_min_max_measure_) {
          data1 -= (min + max);
        }
        data1 /= count_measure_;
      }
    }

    if (!in_range(data1)) {
      has_touch = false;
    }
    else if (ctrl == x_ctrl) { // X Measure done
      raw_x_ = data1;
      ctrl = y_ctrl; // Y-POSITION Measure
    }
    else if (ctrl == y_ctrl) { // Y Measure done
      raw_y_ = data1;
      if (z_first_measure_) break;
      ctrl = z1_ctrl; // Z1-POSITION Measure
    }
    else if (ctrl == z1_ctrl) { // Z1 Measure done
      if (z_local_min_measure_) { // read z1 until grows
        if (raw_z1_ >= data1) {
          ctrl = z2_ctrl; // Z2-POSITION Measure
          if (count_measure_ > 0) {
            if (!only_z1) spi_.transfer16(ctrl); // dummy read of last z1, because next transfer16() must return z2
          }
        }
      }
      else {
        ctrl = z2_ctrl; // Z2-POSITION Measure
      }
      raw_z1_ = data1;
      if (only_z1 && ctrl == z2_ctrl) {
        if (count_measure_ == 0) {
          spi_.transfer16(OFF_MEASURE); // set power down mode
        }
        spi_end();
        return;
      }
    }
    else if (ctrl == z2_ctrl) { // Z2 Measure done
      raw_z2_ = data1;
      if (!z_first_measure_) break;
      ctrl = x_ctrl; // X-POSITION Measure
    }
  }
  if (count_measure_ == 0) {
    spi_.transfer16(OFF_MEASURE); // set power down mode
  }
  spi_end();

#ifdef TFT_TOUCH_SERIAL_DEBUG_FETCH //
  if (!has_touch) {
    Serial.print("raw measure out of range value: "); Serial.print(data1); Serial.print(" ctrl: 0x"); Serial.println(ctrl, HEX);
  }
#endif

  if (has_touch && raw_z1_ > 0) { // if z1 is 0 we get a division by 0 exception!
    if (raw_z1_ >= raw_z2_) rz_ = 0;  // more then 2 Finger
//    else rz_ = (uint16_t)((((int32_t)rx_plate_ * raw_z2_ / raw_z1_) * raw_x_ / 4096) - (int32_t)rx_plate_ * raw_x_ / 4096);
    else rz_ = (uint16_t)(((((int32_t)rx_plate_ * raw_z2_ / raw_z1_) * raw_x_) - (int32_t)rx_plate_ * raw_x_) / 4096);
    // Formula from ADS7846 pdf: R_TOUCH = Rx-plate * X-Position/4096 * (Z2/Z1 - 1) ; bud work only with float bud i prefer speed
//    else rz_ = (uint16_t)((float)rx_plate_ * raw_x_ / 4096.0 * ((float)raw_z2_ / raw_z1_ - 1.0));

//    Serial.print("rz: "); Serial.print((uint16_t)((((int32_t)rx_plate_ * raw_z2_ / raw_z1_) * raw_x_ / 4096) - (int32_t)rx_plate_ * raw_x_ / 4096));
//    Serial.print(", "); Serial.print((uint16_t)(((((int32_t)rx_plate_ * raw_z2_ / raw_z1_) * raw_x_) - (int32_t)rx_plate_ * raw_x_) / 4096));
//    Serial.print(", "); Serial.print((uint16_t)((float)rx_plate_ * raw_x_ / 4096.0 * ((float)raw_z2_ / raw_z1_ - 1.0))); Serial.print(" ");
  }
  else rz_ = 0xffff;  // indicate 'no touch'
}

// -- 
void TFT_eTouchBase::CalibrationPoint::print()
{
#ifdef TFT_TOUCH_SERIAL_DEBUG
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
