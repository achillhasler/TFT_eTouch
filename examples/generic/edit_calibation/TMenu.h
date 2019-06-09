#ifndef T_MENU_H
#define T_MENU_H

//
//  TMenu.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//

#define TEST_RAW_INTERFACE

#include <TFT_eTouch.h>
#include "MenuCounter.h"

class TMenu
{
  TFT_eTouch<TFT_Driver>& touch_;

  MenuCounter mc_[4];
  uint8_t offset_;
#ifdef TEST_RAW_INTERFACE
  TFT_eTouchBase::Measure raw_;
  TFT_eTouchBase::TouchPoint tp_;
#else
  int16_t x_;
  int16_t y_;
  uint16_t rz_;
#endif
  bool touched_;
  bool cursor_visible_;
  

  void draw();
  void info(uint8_t offset);

  public:
  TMenu(TFT_eTouch<TFT_Driver>& touch, uint8_t offset = 18);
  
  TFT_Driver& tft();

  EventType pen_up();
  EventType pen_down();
  
  void init();
#ifdef TEST_RAW_INTERFACE
  void update(const TFT_eTouchBase::Measure& raw);
#else
  void update(int16_t x, int16_t y, uint16_t rz);
#endif
  void refresh();
};

#endif // T_MENU_H
