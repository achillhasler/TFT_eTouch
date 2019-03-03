//
//  TMenu.cpp
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//

#include "TMenu.h"

TMenu::TMenu(TFT_eTouch<TFT_eSPI>& touch, uint8_t offset)
: touch_(touch)
, offset_(offset)
#ifndef TEST_RAW_INTERFACE
, x_(0)
, y_(0)
, rz_(0)
#endif
, touched_(false)
, cursor_visible_(false)
{
}

TFT_eSPI& TMenu::tft()
{
  return touch_.tft();
}

EventType TMenu::pen_up()
{
  EventType ret = none;
  touched_ = false;
  for (uint8_t i = 0; i < 4 ; i++) {
    ret = mc_[i].signal();
    mc_[i].pen_up();
    if (ret > changed) break;
  }
#ifdef TEST_RAW_INTERFACE
  tp_.rz++;
#else
  rz_++; // change a little bit, for hiding cursor in next refresh call
#endif
  return ret;
}

EventType TMenu::pen_down()
{
  touched_ = true;
  return none;
}

void TMenu::init()
{
  TFT_eTouchBase::Calibation& calibation = touch_.calibration();
  uint8_t d_rot = (4 + tft().getRotation() - calibation.rel_rotation) % 4;

//  Serial.printf("d_rot %d\n", d_rot);

  for (uint8_t i = 0; i < 4 ; i++) {
    mc_[i].oner(this);
  }
  // pos from d_rot == 0
  mc_[0].pos(offset_, (tft().height() - 5 * mc_[0].fw()) / 2, false);
  mc_[1].pos((tft().width() - 5 * mc_[1].fw()) / 2, offset_, true);
  mc_[2].pos(tft().width() - 1 - offset_ - mc_[2].fw(), (tft().height() - 5 * mc_[2].fw()) / 2, false);
  mc_[3].pos((tft().width() - 5 * mc_[3].fw())/2, tft().height() - 1 - offset_ - mc_[3].fw(), true);
  
  mc_[(4 + 0 - d_rot) % 4].cmd('X', none);  // do nothing
  mc_[(4 + 0 - d_rot) % 4].val(calibation.x0);

  mc_[(4 + 1 - d_rot) % 4].cmd('Y', none);  // do nothing
  mc_[(4 + 1 - d_rot) % 4].val(calibation.y0);

  mc_[(4 + 2 - d_rot) % 4].cmd('S', store);  // store calibation on pen up
  mc_[(4 + 2 - d_rot) % 4].val(calibation.x1);

  mc_[(4 + 3 - d_rot) % 4].cmd('C', calibrate);  // calibrate on pen up
  mc_[(4 + 3 - d_rot) % 4].val(calibation.y1);

  // Clear the screen
  tft().fillScreen(TFT_BLACK);

#ifdef BASIC_FONT_SUPPORT
  tft().setTextSize(1);
  char* str;
  str = "Touch screen!";
  int16_t len = strlen(str) * 6;
  tft().setCursor((tft().width() - len)/2, tft().height()/2 - 30);
  tft().print(str);

  str = "C: calibrate";
  len = strlen(str) * 6;
  tft().setCursor((tft().width()-len)/2 , tft().height()/2 - 10);
  tft().print(str);

  str = "S: store";
  len = strlen(str) * 6;
  tft().setCursor((tft().width()-len)/2 , tft().height()/2 + 10);
  tft().print(str);

  str = "(on pen up)";
  len = strlen(str) * 6;
  tft().setCursor((tft().width()-len)/2 , tft().height()/2 + 30);
  tft().print(str);
#else
  tft().setTextFont(2);
  tft().setTextDatum(TC_DATUM);
  tft().drawString("Touch screen!", tft().width()/2, tft().height()/2 - 30, 2);
  tft().drawString("C: calibrate", tft().width()/2, tft().height()/2 - 10, 2);
  tft().drawString("S: store", tft().width()/2, tft().height()/2 + 10, 2);
  tft().drawString("(on pen up)", tft().width()/2, tft().height()/2 + 30, 2);
  tft().setTextDatum(TL_DATUM);
#endif

  refresh();
}

void TMenu::draw()
{ 
  for (uint8_t i = 0; i < 4 ; i++) {
    mc_[i].draw();
  }
}

void TMenu::info(uint8_t offset)
{ 
#ifdef TEST_RAW_INTERFACE
  tft().setCursor(offset, offset);
  tft().printf("Pos: %d, %d, %d ", tp_.x, tp_.y, tp_.rz);

  if (tp_.rz > touch_.getRXPlate()*174/100)  tft().println("Pen                 ");
  else if (tp_.rz >= touch_.getRXPlate()*66/100) tft().println("Finger              ");
  else if (tp_.rz > touch_.getRXPlate()*15/100) tft().println("2 Finger            ");
  else tft().println("+ 2 Finger        ");

  tft().setCursor(offset, tft().getCursorY());
  tft().printf("Raw: %d, %d, %d,%d           \n", raw_.x, raw_.y, raw_.z1, raw_.z2);
#else
  uint16_t xr;
  uint16_t yr;
  uint16_t z1;
  uint16_t z2;
  uint16_t rz;
  touch_.getRaw(xr, yr, z1, z2, rz);

  tft().setCursor(offset, offset);
  tft().printf("Pos: %d, %d, %d ", x_, y_, rz_);
  if (rz_ > touch_.getRZThreshold()*58/100)  tft().println("Pen                 ");
  else if (rz_ >= touch_.getRZThreshold()*22/100) tft().println("Finger              ");
  else if (rz_ > touch_.getRZThreshold()*5/100) tft().println("2 Finger            ");
  else tft().println("+ 2 Finger        ");

  tft().setCursor(offset, tft().getCursorY());
  tft().printf("Raw: %d, %d, %d,%d (%s)       \n", xr, yr, z1, z2, (rz == rz_) ? "same" : "new" );

#endif
  tft().setCursor(offset, tft().getCursorY());
  TFT_eTouchBase::Calibation& calibation = touch_.calibration();
  tft().printf("Cal: %d,%d, %d,%d, %d     \n", calibation.x0, calibation.x1, calibation.y0, calibation.y1, calibation.rel_rotation);
}

#ifdef TEST_RAW_INTERFACE
void TMenu::update(const TFT_eTouchBase::Measure& raw)
{
  if (touch_.transform(raw, tp_)) {
    raw_ = raw;
    for (uint8_t i = 0; i < 4 ; i++) {
      mc_[i].update(tp_.x, tp_.y, tp_.rz);
    }
  }
}
#else
void TMenu::update(int16_t x, int16_t y, uint16_t rz)
{
  x_ = x;
  y_ = y;
  rz_ = rz;

  for (uint8_t i = 0; i < 4 ; i++) {
    mc_[i].update(x, y, rz);
  }
}
#endif


void TMenu::refresh()
{
  const uint8_t cursor_size = 8, cursor_hole = 3;
  bool same_pos = false;

#ifdef TEST_RAW_INTERFACE
  static TFT_eTouchBase::TouchPoint last;

  if (last.x == tp_.x && last.y == tp_.y) {
    if (last.rz == tp_.rz) return;
    same_pos = true;
  }

  if (cursor_visible_ && (!same_pos || !touched_)) {
    tft().drawFastHLine(last.x - cursor_size + 1, last.y, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastHLine(last.x + cursor_hole, last.y, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastVLine(last.x, last.y - cursor_size + 1, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastVLine(last.x, last.y + cursor_hole, cursor_size - cursor_hole, TFT_BLACK);  
    tft().drawPixel(last.x, last.y, TFT_BLACK);  
  }

  draw();
  if (!touched_) {
    cursor_visible_ = false;  // hide cursor
    return;
  }
  
  info(50);

  if (!same_pos) {
    tft().drawFastHLine(tp_.x - cursor_size + 1, tp_.y, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastHLine(tp_.x + cursor_hole, tp_.y, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastVLine(tp_.x, tp_.y - cursor_size + 1, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastVLine(tp_.x, tp_.y + cursor_hole, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawPixel(tp_.x, tp_.y, TFT_WHITE); 
  }
  last = tp_;
#else
  static int16_t vx = 10000, vy = 10000, vz = 10000;

  if (vx == x_ && vy == y_) {
    if (vz == rz_) return;
    same_pos = true;
  }
  if (cursor_visible_ && (!same_pos || !touched_)) {
    tft().drawFastHLine(vx - cursor_size + 1, vy, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastHLine(vx + cursor_hole, vy, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastVLine(vx, vy - cursor_size + 1, cursor_size - cursor_hole, TFT_BLACK);
    tft().drawFastVLine(vx, vy + cursor_hole, cursor_size - cursor_hole, TFT_BLACK);  
    tft().drawPixel(vx, vy, TFT_BLACK);  
  }

  draw();
  if (!touched_) {
    cursor_visible_ = false;  // hide cursor
    return;
  }
  
  info(50);

  if (!same_pos) {
    tft().drawFastHLine(x_ - cursor_size + 1, y_, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastHLine(x_ + cursor_hole, y_, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastVLine(x_, y_ - cursor_size + 1, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawFastVLine(x_, y_ + cursor_hole, cursor_size - cursor_hole, TFT_WHITE);
    tft().drawPixel(x_, y_, TFT_WHITE); 
  }
  vx = x_; vy = y_; vz = rz_;
#endif
  cursor_visible_ = true;
}
