#ifndef TFT_E_TOUCH_GESTURE_H
#define TFT_E_TOUCH_GESTURE_H

//
//  TFT_eTouchGesture.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

#include <TFT_eTouchBase.h>

/** 
  * @brief  Gesture support for touch 
  */
class TFT_eTouchGesture
{
  struct FilteredMeasure
  {
    uint8_t    x; ///< raw x measure reduced to 8 bit (ca 213..3881)/16
    uint8_t    y; ///< raw y measure reduced to 8 bit (ca 222..3929)/16
    uint8_t    z1; ///< raw z1 measure reduced to 8 bit (ca 29..1748)/16
    uint8_t    z2; ///< raw z2 measure reduced to 8 bit (ca 963..3989)/16
    uint8_t    rz; ///< calculated resitor value < 0xff when tuched, 0xff when not tuched.
    uint16_t   ms;
    FilteredMeasure() : rz(0xff) {}
    void operator=(const TFT_eTouchBase::Measure& raw);
  };

public:
  typedef enum
  {
    none,
    stay,
    move,
    wipe,
    zoom_in,
    zoom_out
  } Action;

/** 
  * @brief  constructor
  * @param  size stored values
  */
            TFT_eTouchGesture(uint16_t size);
            ~TFT_eTouchGesture();

/** 
  * You have to feed this instance with new measure in a constant timeframe.
  * @brief  set measure
  * @param  raw last readed touch measure
  */
  void      set(const TFT_eTouchBase::Measure& raw);
  
/** 
  * On pen up reset must call.
  * @brief  reset
  */
  void      reset();
  
/** 
  * Get the action. When Action move or wipe is returned the angle is valid.
  * @brief  get action
  * @param  angle in grad
  * @return actual action
  */
  Action    get(int16_t& angle);
  
private:
  FilteredMeasure* data_;

  uint16_t size_;
  uint16_t next_;
  uint16_t used_;
};



//#include <TFT_eTouchGesture.inl>

#endif // TFT_E_TOUCH_GESTURE_H
