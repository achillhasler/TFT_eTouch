#ifndef TFT_E_TOUCH_H
#define TFT_E_TOUCH_H

//
//  TFT_eTouch.inl
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at http://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

#include <TFT_eTouchBase.h>

/** 
  * @brief  touch support for tft
  * @param T used display driver, must be Adafuit compatible
  */
template <class T>
class TFT_eTouch : public TFT_eTouchBase
{
public:
/** 
  * Create instance with defaults.
  *
  * Display and touch must use the same spi bus.
  * @brief constructor
  * @param tft used display, must be Adafuit compatible
  * @param cs_pin touch chip select line prcessor pin
  * @param penirq_pin touch penirq line prcessor pin. 0xff disable touch interrupt
  * @param spi used bus
  */
            TFT_eTouch(T& tft, uint8_t cs_pin, uint8_t penirq_pin = 0xff, SPIClass& spi = SPI);

/** 
  * Initialize the processor pins and initialize interrupt handling.
  * @brief  init touch
  */
	void      init();

/** 
  * Calculate the calibration values from 4 measures. User has to hit the blue target. 
  * If it is get red try it again til it is green and next target is shown.
  * At the end a configuration string will be printed out to Serial.
  * @brief  get calibration from user
  * @param data display coordinate of touch
  * @param xy_off display coordinate of touch
  * @return false on error otherwise data is set.
  */
  bool      getCalibration(Calibation& data, uint8_t xy_off);

/** 
  * Get display position of touch.
  * The values x and y are only set if the function returns true.
  * @brief  display position
  * @param x display coordinate of touch
  * @param y display coordinate of touch
  * @return true when display is tuched
  */
  bool      getXY(int16_t& x, int16_t& y);
  
#ifdef TOUCH_USE_PENIRQ_CODE
#ifdef ESP32
  static void IRAM_ATTR cb_isr_touch_fnk();
#else
  static void cb_isr_touch_fnk();
#endif
#endif // end TOUCH_USE_PENIRQ_CODE

private:
/** 
  * Show one calibration target and get the raw touch values for that point.
  * The Target get red when not accurate and function returns false.
  * othewise the target get green and function returns true.
  *
  * If it is allways get red try a other configuration. see setAcurateDistance()
  * @brief one calibration target
  * @param point target, when the function returns true touch parameters are set.   
  * @return true when accurate
  */
  bool      handleTouchCalibrationTarget(CalibrationPoint& point);

  T& tft_; ///< the given display driver
#ifdef TOUCH_USE_PENIRQ_CODE
  static TFT_eTouch<T>* isr_instance_; ///< one instance used for interrupt handling
#endif // end TOUCH_USE_PENIRQ_CODE
};

#include <TFT_eTouch.inl>

#endif // TFT_E_TOUCH_H
