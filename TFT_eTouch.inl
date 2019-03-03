#ifndef TFT_E_TOUCH_INL
#define TFT_E_TOUCH_INL

//
//  TFT_eTouch.inl
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

#ifdef TOUCH_USE_PENIRQ_CODE
template <class T>
TFT_eTouch<T>* TFT_eTouch<T>::isr_instance_ = 0;

template <class T>
#ifdef ESP32
void IRAM_ATTR TFT_eTouch<T>::cb_isr_touch_fnk()
#else
void TFT_eTouch<T>::cb_isr_touch_fnk()
#endif
{
	TFT_eTouch<T>* touch_ptr = isr_instance_;
  if (touch_ptr) touch_ptr->update_allowed_ = true;
}
#endif // end TOUCH_USE_PENIRQ_CODE


template <class T>
TFT_eTouch<T>::TFT_eTouch(T& tft, uint8_t cs_pin, uint8_t penirq_pin, SPIClass& spi)
: TFT_eTouchBase(cs_pin, penirq_pin, spi)
, tft_(tft)
{
}

template <class T>
void TFT_eTouch<T>::init()
{
  TFT_eTouchBase::init(false);
#ifdef TOUCH_USE_PENIRQ_CODE
  if (penirq_ != 0xff) {
    pinMode(penirq_, INPUT);
    attachInterrupt(digitalPinToInterrupt(penirq_), cb_isr_touch_fnk, FALLING);
    isr_instance_ = this;
  }
#endif // end TOUCH_USE_PENIRQ_CODE
}

template <class T>
T& TFT_eTouch<T>::tft()
{
  return tft_;
}

#ifdef TOUCH_USE_USER_CALIBRATION
template <class T>
bool TFT_eTouch<T>::getUserCalibration(Calibation& data, uint8_t xy_off)
{
  uint8_t user_r = tft_.getRotation();
  uint8_t touch_r = 255;
  
  tft_.fillScreen(TFT_BLACK);
  tft_.setCursor(50, 50);
#ifndef BASIC_FONT_SUPPORT
  tft_.setTextFont(2);
#endif
  tft_.setTextSize(1);
  tft_.setTextColor(TFT_WHITE, TFT_BLACK);

  tft_.println("Touch blue target to green");

#ifndef BASIC_FONT_SUPPORT
  tft_.setTextFont(1);
#endif
  tft_.println();

  if (xy_off < 4) xy_off = 4;
  
  CalibrationPoint cal_pos[4]; // get clockwise 4 values 
  cal_pos[0].set(xy_off, xy_off);  // Top Left
  cal_pos[1].set(tft_.width() - 1 - xy_off, xy_off);  // Top Right
  cal_pos[2].set(tft_.width() - 1 - xy_off, tft_.height() - 1 - xy_off); // Down Right
  cal_pos[3].set(xy_off, tft_.height() - 1 - xy_off);  // Down Left
  
  while (!handleTouchCalibrationTarget(cal_pos[0])) { delay(1000); }
  while (!handleTouchCalibrationTarget(cal_pos[1])) { delay(1000); }
  while (!handleTouchCalibrationTarget(cal_pos[2])) { delay(1000); }
  while (!handleTouchCalibrationTarget(cal_pos[3])) { delay(1000); }

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("p0 "); cal_pos[0].print();
  Serial.print("p1 "); cal_pos[1].print();
  Serial.print("p2 "); cal_pos[2].print();
  Serial.print("p3 "); cal_pos[3].print();
#endif

  for (int i = 0; i < 4; i++) {
    if (cal_pos[i].touch_x < 1000 && cal_pos[i].touch_y < 1000) {
      // zerro of touch, is touch_y inverted ?
      if (cal_pos[(i+4-1) % 4].touch_x < 1000) {
        // not inverted
        touch_r = i;
      }
      else {
        touch_r = (i + 1) % 4;
      }
      break;
    }
  }

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("Rotation: tft ");
  Serial.print(user_r);
  Serial.print(" touch ");
  Serial.println(touch_r);
#endif

  int16_t d_xs = cal_pos[1].scr_x - cal_pos[0].scr_x;
  int16_t d_xt = (cal_pos[(touch_r + 1) % 4].touch_x + cal_pos[(touch_r + 2) % 4].touch_x)/2 - (cal_pos[touch_r + 0].touch_x + cal_pos[(touch_r + 3) % 4].touch_x)/2;

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("dx ");
  Serial.print(d_xs);
  Serial.print(", ");
  Serial.print(d_xt);
#endif
  
  int16_t x0 = cal_pos[0].scr_x - d_xs * (cal_pos[touch_r + 0].touch_x+cal_pos[(touch_r + 3) % 4].touch_x) / (d_xt * 2);
#ifdef TOUCH_SERIAL_DEBUG
  Serial.print(", ");
  Serial.println(x0);
#endif
  
  data.x0 = -x0 * d_xt / d_xs;
  data.x1 = (tft_.width() - 1 - x0) * d_xt / d_xs;
  
  int16_t d_ys = cal_pos[3].scr_y - cal_pos[0].scr_y;
  int16_t d_yt = (cal_pos[(touch_r + 2) % 4].touch_y + cal_pos[(touch_r + 3) % 4].touch_y)/2 - (cal_pos[touch_r + 0].touch_y + cal_pos[(touch_r + 1) % 4].touch_y)/2;

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("dy ");
  Serial.print(d_ys);
  Serial.print(", ");
  Serial.print(d_yt);
#endif

  int16_t y0 = cal_pos[0].scr_y - d_ys * (cal_pos[touch_r + 0].touch_y+cal_pos[(touch_r + 1) % 4].touch_y) / (d_yt * 2);

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print(", ");
  Serial.println(y0);
#endif
  
  data.y0 = -y0 * d_yt / d_ys;
  data.y1 = (tft_.height() - 1 - y0) * d_yt / d_ys;

  data.rel_rotation = (user_r + touch_r) % 4;
  
#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("x: "); Serial.print(data.x0); Serial.print(" "); Serial.print(data.x1);
  Serial.print(" y: "); Serial.print(data.y0); Serial.print(" "); Serial.print(data.y1);
  Serial.print(" rot "); Serial.println(data.rel_rotation);
#endif

#ifdef TOUCH_SERIAL_DEBUG
  Serial.print("TFT_eTouchBase::Calibation calibation = { ");
  Serial.print(data.x0); Serial.print(", "); 
  Serial.print(data.x1); Serial.print(", ");
  Serial.print(data.y0); Serial.print(", "); 
  Serial.print(data.y1); Serial.print(", "); 
  Serial.print(data.rel_rotation); Serial.println(" };"); 
#endif

  return true;
}
#endif // TOUCH_USE_USER_CALIBRATION

template <class T>
bool TFT_eTouch<T>::transform(const Measure& raw, TouchPoint& tp)
{
  if (raw.rz != 0xffff) {
    uint8_t d_rot = (4 + tft_.getRotation() - calibation_.rel_rotation) % 4;

    int16_t dx = calibation_.x1 - calibation_.x0;
    int16_t dy = calibation_.y1 - calibation_.y0;

    int16_t xs, xs_max = tft_.width() - 1;
    int16_t ys, ys_max = tft_.height() - 1;

    switch (d_rot) {
    case 0:
      xs = (raw.x - calibation_.x0) * xs_max / dx;
      ys = (raw.y - calibation_.y0) * ys_max / dy;
      break;
    case 1:
      xs = (raw.y - calibation_.y0) * xs_max / dy;
      ys = ys_max - ((raw.x - calibation_.x0) * ys_max / dx);
      break;
    case 2:
      xs = xs_max - ((raw.x - calibation_.x0) * xs_max / dx);
      ys = ys_max - ((raw.y - calibation_.y0) * ys_max / dy);
      break;
    case 3:
      xs = xs_max - ((raw.y - calibation_.y0) * xs_max / dy);
      ys = (raw.x - calibation_.x0) * ys_max / dx;
      break;
    }
    if (xs < 0) xs = 0;
    if (xs > xs_max) xs = xs_max;

    if (ys < 0) ys = 0;
    if (ys > ys_max) ys = ys_max;
    
    tp.set(xs, ys, raw.rz);
    return true;
  }
  tp.rz = 0xffff;
  return false;
}

template <class T>
bool TFT_eTouch<T>::getXY(int16_t& x, int16_t& y)
{
  update(false);
  if (valid()) {
    TouchPoint tp;
    transform(raw_, tp);
    x = tp.x;
    y = tp.y;

    return true;
  }
  return false;
}

template <class T>
bool TFT_eTouch<T>::get(TouchPoint& tp)
{
  update(false);
  if (valid()) {
    return transform(raw_, tp);
  }
  return false;
}

// private
#ifdef TOUCH_USE_USER_CALIBRATION

#ifdef TOUCH_USE_SIMPE_TARGET
template <class T>
bool TFT_eTouch<T>::handleTouchCalibrationTarget(CalibrationPoint& point)
{
  bool acurate = false;
  const uint8_t size = 10, hole = 3;
  uint16_t sum_x = 0, sum_y = 0;
  uint16_t max_x = 0, max_y = 0;
  uint16_t min_x = 0xffff, min_y = 0xffff;

  tft_.fillRect(point.scr_x - 4, point.scr_y - 4, 9, 9, TFT_BLUE);
  tft_.fillCircle(point.scr_x, point.scr_y, 4, TFT_WHITE);
  tft_.fillCircle(point.scr_x, point.scr_y, 2, TFT_BLUE);

  delay(500);
  uint8_t cnt = 0;
  uint16_t x, y, z;
  uint16_t org_wait = getMeasureWait();
  setMeasureWait(0);
  while (cnt < 16) {
    update(false);
    if (valid()) {
      sum_x += raw_.x; sum_y += raw_.y;
      if (max_x < raw_.x) max_x = raw_.x;
      if (max_y < raw_.y) max_y = raw_.y;
      if (min_x > raw_.x) min_x = raw_.x;
      if (min_y > raw_.y) min_y = raw_.y;
      cnt++;
    }
  }
  setMeasureWait(org_wait);
  if ( (max_x - min_x <= getAcurateDistance()) && (max_y - min_y <= getAcurateDistance()) ) {
    acurate = true;
    point.touch_x = sum_x / 16;
    point.touch_y = sum_y / 16;
  }

  tft_.fillRect(point.scr_x - 4, point.scr_y - 4, 9, 9, acurate ? TFT_GREEN : TFT_RED);
  tft_.fillCircle(point.scr_x, point.scr_y, 4, TFT_WHITE);
  tft_.fillCircle(point.scr_x, point.scr_y, 2, acurate ? TFT_GREEN : TFT_RED);

  // wait pen up
  waitPenUp();
  delay(500);
  
  return acurate;
}
#else // !TOUCH_USE_SIMPE_TARGET
template <class T>
bool TFT_eTouch<T>::handleTouchCalibrationTarget(CalibrationPoint& point)
{
  bool acurate = false;
  const uint8_t size = 10, hole = 3;
  uint16_t sum_x = 0, sum_y = 0;
  uint16_t max_x = 0, max_y = 0;
  uint16_t min_x = 0xffff, min_y = 0xffff;

  tft_.fillCircle(point.scr_x, point.scr_y, 10, TFT_WHITE);
  tft_.fillCircle(point.scr_x, point.scr_y, 9, TFT_BLUE);
  tft_.fillCircle(point.scr_x, point.scr_y, 4, TFT_WHITE);
  tft_.fillCircle(point.scr_x, point.scr_y, 2, TFT_BLACK);
  if (point.scr_x < size || point.scr_y < size || point.scr_x > tft_.width() - size || point.scr_y > tft_.height() - size) {
    // fillCircle won't work if not full visible
    tft_.drawCircle(point.scr_x, point.scr_y, 3, TFT_WHITE);
    tft_.drawCircle(point.scr_x, point.scr_y, 4, TFT_WHITE);
    tft_.drawCircle(point.scr_x, point.scr_y, 5, TFT_BLUE);
    tft_.drawCircle(point.scr_x, point.scr_y, 6, TFT_BLUE);
    tft_.drawCircle(point.scr_x, point.scr_y, 7, TFT_BLUE);
    tft_.drawCircle(point.scr_x, point.scr_y, 8, TFT_BLUE);
    tft_.drawCircle(point.scr_x, point.scr_y, 9, TFT_BLUE);
    tft_.drawCircle(point.scr_x, point.scr_y, 10, TFT_WHITE);
  }
  tft_.drawFastHLine(point.scr_x - size + 1, point.scr_y, size - hole, TFT_WHITE);
  tft_.drawFastHLine(point.scr_x + hole, point.scr_y, size - hole, TFT_WHITE);
  tft_.drawFastVLine(point.scr_x, point.scr_y - size + 1, size - hole, TFT_WHITE);
  tft_.drawFastVLine(point.scr_x, point.scr_y + hole, size - hole, TFT_WHITE);

  delay(500);
  uint8_t cnt = 0;
  uint16_t x, y, z;
  uint16_t org_wait = getMeasureWait();
  setMeasureWait(0);
  while (cnt < 16) {
    update(false);
    if (valid()) {
      sum_x += raw_.x; sum_y += raw_.y;
      if (max_x < raw_.x) max_x = raw_.x;
      if (max_y < raw_.y) max_y = raw_.y;
      if (min_x > raw_.x) min_x = raw_.x;
      if (min_y > raw_.y) min_y = raw_.y;
      cnt++;
    }
  }
  setMeasureWait(org_wait);
  if ( (max_x - min_x <= getAcurateDistance()) && (max_y - min_y <= getAcurateDistance()) ) {
    acurate = true;
    point.touch_x = sum_x / 16;
    point.touch_y = sum_y / 16;
  }

  tft_.fillCircle(point.scr_x, point.scr_y, 9, acurate ? TFT_GREEN : TFT_RED);
  tft_.fillCircle(point.scr_x, point.scr_y, 4, TFT_WHITE);
  tft_.fillCircle(point.scr_x, point.scr_y, 2, TFT_BLACK);
  if (point.scr_x < size || point.scr_y < size || point.scr_x > tft_.width() - size || point.scr_y > tft_.height() - size) {
    tft_.drawCircle(point.scr_x, point.scr_y, 5, acurate ? TFT_GREEN : TFT_RED);
    tft_.drawCircle(point.scr_x, point.scr_y, 6, acurate ? TFT_GREEN : TFT_RED);
    tft_.drawCircle(point.scr_x, point.scr_y, 7, acurate ? TFT_GREEN : TFT_RED);
    tft_.drawCircle(point.scr_x, point.scr_y, 8, acurate ? TFT_GREEN : TFT_RED);
    tft_.drawCircle(point.scr_x, point.scr_y, 9, acurate ? TFT_GREEN : TFT_RED);
  }
  tft_.drawFastHLine(point.scr_x - size + 1, point.scr_y, size - hole, TFT_WHITE);
  tft_.drawFastHLine(point.scr_x + hole, point.scr_y, size - hole, TFT_WHITE);
  tft_.drawFastVLine(point.scr_x, point.scr_y - size + 1, size - hole, TFT_WHITE);
  tft_.drawFastVLine(point.scr_x, point.scr_y + hole, size - hole, TFT_WHITE);
  // wait pen up
  waitPenUp();
  delay(500);
  
  return acurate;
}
#endif // TOUCH_USE_SIMPE_TARGET
#endif // TOUCH_USE_USER_CALIBRATION

#endif // TFT_E_TOUCH_INL

