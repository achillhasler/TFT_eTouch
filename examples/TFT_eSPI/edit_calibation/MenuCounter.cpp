//
//  TMenuCounter.cpp
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//

#include "TMenu.h"

void MenuCounter::draw_move(int16_t x_pos, int16_t y_pos, bool flag)
{
  if (x_grow_) {
    if (flag) {
      // up
      // menu_->tft().
      tft().drawLine(x_pos + fw_*4/16, y_pos, x_pos + fw_*8/16, y_pos + fw_*4/16, TFT_WHITE);
      tft().drawLine(x_pos + fw_*8/16, y_pos + fw_*4/16, x_pos + fw_*12/16, y_pos, TFT_WHITE);
    }
    else {
      // down
      tft().drawLine(x_pos + fw_*4/16, y_pos, x_pos + fw_*8/16, y_pos - fw_*4/16, TFT_WHITE);
      tft().drawLine(x_pos + fw_*8/16, y_pos - fw_*4/16, x_pos + fw_*12/16, y_pos, TFT_WHITE);
    }
  }
  else {
    if (flag) {
      // right
      tft().drawLine(x_pos, y_pos + fw_*4/16, x_pos + fw_*4/16, y_pos + fw_*8/16, TFT_WHITE);
      tft().drawLine(x_pos + fw_*4/16, y_pos + fw_*8/16, x_pos, y_pos + fw_*12/16, TFT_WHITE);
    }
    else {
      // left
      tft().drawLine(x_pos, y_pos + fw_*4/16, x_pos - fw_*4/16, y_pos + fw_*8/16, TFT_WHITE);
      tft().drawLine(x_pos - fw_*4/16, y_pos + fw_*8/16, x_pos, y_pos + fw_*12/16, TFT_WHITE);
    }
  }
}


MenuCounter::MenuCounter(uint8_t fw)
: menu_(0)
, fw_(fw)
, cnt_step_(12)
, activ_(255)
, last_activ_(255)
, ch_signal_(none)
, signal_(none)
, val_(0)
{}

void MenuCounter::oner(TMenu* menu)
{
  menu_ = menu;
}

TFT_eSPI& MenuCounter::tft() const
{
  return menu_->tft();
}

void MenuCounter::pen_up()
{
  activ_ = 255;
  if (last_activ_ < 5) draw();
}

void MenuCounter::update(int16_t x, int16_t y, uint16_t rz)
{
  static uint32_t last_update = 0;
  
  bool inside = false;
  if ((x > x_p_) && (y > y_p_)) {
    x -= x_p_;
    y -= y_p_;
    if (x_grow_) {
      if ((x < (5 * fw_)) && (y < (1 * fw_))) inside = true;
    }
    else {
      if ((x < (1 * fw_)) && (y < (5 * fw_))) inside = true;
    }
  }
  if (!inside) {
    activ_ = 255;
    if (signal_ != none) {
      signal_ = none;
      draw();
    }
    return;
  }
  uint8_t ind;
  if (x_grow_) {
    ind = x / fw_;
  }
  else {
    ind = y / fw_;
  }
  if (last_update + 500 < millis()) {
    last_update = millis();
    uint16_t val = *val_;
//    Serial.printf("update %d val: %p %d -> ", ind, val_, val);
    switch (ind) {
      case 0: // decrement var
        if (val - cnt_step_ > 200) val -= cnt_step_;
      break;
      case 1:
        if (val > 200) val--;
      break;
      case 2:
        signal_ = ch_signal_;
      break;
      case 3:
        if (val < 4000) val++;
      break;
      case 4:
        if (val + cnt_step_ < 4000) val += cnt_step_;
      break;
      default:
       return;
    }
    if (ind != 2) {
      signal_ = changed;
      *val_ = val;
//      Serial.println(val);
    }
  }
  activ_ = ind;
}

void MenuCounter::draw()
{
  uint16_t i;

  if (activ_ == 2) tft().setTextColor(TFT_WHITE, TFT_BLUE);

  if (x_grow_) {
    if (last_activ_ < 5 && activ_ != last_activ_) {
      tft().fillRect(x_p_ + 1 + last_activ_ * fw_, y_p_ + 1, fw_-1, fw_-1, TFT_BLACK);
    }
    for (i = 0; i <= 5; i++) {
      tft().drawFastVLine(x_p_ + i * fw_, y_p_ + 1, fw_ - 1, TFT_WHITE);
    }
    tft().drawFastHLine(x_p_, y_p_, 5 * fw_ + 1, TFT_WHITE);
    tft().drawFastHLine(x_p_, y_p_ + fw_, 5 * fw_ + 1, TFT_WHITE);
    if (activ_ < 5) {
      tft().fillRect(x_p_ + 1 + activ_ * fw_, y_p_ + 1, fw_-1, fw_-1, TFT_BLUE);
    }
    draw_move(x_p_, y_p_ + fw_*4/16, true);
    draw_move(x_p_, y_p_ + fw_*8/16, true);

    draw_move(x_p_ + fw_, y_p_ + fw_*6/16, true);

    tft().setCursor(x_p_ + 2*fw_ + 6, y_p_ + 3);
    tft().print(ch_);

    draw_move(x_p_ + 3*fw_, y_p_ + fw_*10/16, false);

    draw_move(x_p_ + 4*fw_, y_p_ + fw_*8/16, false);
    draw_move(x_p_ + 4*fw_, y_p_ + fw_*12/16, false);
  }
  else {
    if (last_activ_ < 5 && activ_ != last_activ_) {
      tft().fillRect(x_p_ + 1, y_p_ + 1 + last_activ_ * fw_, fw_-1, fw_-1, TFT_BLACK);
    }
    for (i = 0; i <= 5; i++) {
      tft().drawFastHLine(x_p_ + 1, y_p_ + i * fw_, fw_ - 1, TFT_WHITE);
    }
    tft().drawFastVLine(x_p_, y_p_, 5 * fw_ + 1, TFT_WHITE);
    tft().drawFastVLine(x_p_ + fw_, y_p_, 5 * fw_ + 1, TFT_WHITE);
    if (activ_ < 5) {
      tft().fillRect(x_p_ + 1, y_p_ + 1 + activ_ * fw_, fw_-1, fw_-1, TFT_BLUE);
    }

    draw_move(x_p_ + fw_*8/16, y_p_, false);
    draw_move(x_p_ + fw_*12/16, y_p_, false);

    draw_move(x_p_ + fw_*10/16, y_p_ + fw_, false);

    tft().setCursor(x_p_ + 6, y_p_ + 2*fw_ + 3);
    tft().print(ch_);

    draw_move(x_p_ + fw_*6/16, y_p_ + 3*fw_, true);

    draw_move(x_p_ + fw_*4/16, y_p_ + 4*fw_, true);
    draw_move(x_p_ + fw_*8/16, y_p_ + 4*fw_, true);
  }
  tft().setTextColor(TFT_WHITE, TFT_BLACK);

  last_activ_ = activ_;
}
