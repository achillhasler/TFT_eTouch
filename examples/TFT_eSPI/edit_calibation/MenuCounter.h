#ifndef MENU_COUNTER_H
#define MENU_COUNTER_H

//
//  TMenuCounter.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//

#include <Arduino.h>

typedef enum
{
  none,
  changed,
  calibrate,
  store
} EventType;

class TMenu;

class MenuCounter
{
  TMenu* menu_;
  uint8_t fw_;
  int16_t x_p_;
  int16_t y_p_;
  bool x_grow_;
  
  char ch_;
  EventType ch_signal_;
  EventType signal_;

  uint16_t* val_;

  uint8_t cnt_step_;
  
  uint8_t activ_;
  uint8_t last_activ_;
  
  void draw_move(int16_t x_pos, int16_t y_pos, bool flag);
  
  public:
  MenuCounter(uint8_t fw = 20);

  void oner(TMenu* menu);
  TFT_eSPI& tft() const;

  void pos(int16_t x_p, int16_t y_p, bool horizontal)
  { x_p_ = x_p; y_p_ = y_p; x_grow_ = horizontal; }

  void cmd(char ch, EventType event = none)
  { ch_ = ch; ch_signal_ = event; }

  void val(uint16_t& val)
  { val_ = &val; }

  void fw(uint8_t val) { fw_ = val; }
  uint8_t fw() const { return fw_; }

  EventType signal() { EventType ret = signal_; signal_ = none; return ret; }

  void pen_up();

  void update(int16_t x, int16_t y, uint16_t rz);
  void draw();
};

#endif // MENU_COUNTER_H
