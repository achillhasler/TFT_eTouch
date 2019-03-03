#ifndef TFT_E_TOUCH_USER_H
#define TFT_E_TOUCH_USER_H

//
//  TFT_eTouchUser.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

/** @def TFT_ETOUCH_CS
 * Define the chip select pin for the touch chip.
 */

// include used TFT driver
#ifdef TEENSYDUINO
#include <ILI9341_t3.h>    // Hardware-specific TFT library

#define TFT_ETOUCH_CS 10

// missing setTextFont, drawString in ILI9341_t3 driver
#define BASIC_FONT_SUPPORT

#else // ESP
#include <TFT_eSPI.h>      // Hardware-specific TFT library

#define TFT_ETOUCH_CS 16

#ifdef TOUCH_CS
#error undef TOUCH_CS in TFT_eSPI UserSetup.h for using TFT_eTouch
#endif

#endif

/** @def TOUCH_USE_PENIRQ_CODE
 * If this define is set the penrirq code is used.
 */
// undefine this to save progmem if you not have penirq or not using it
//#define TOUCH_USE_PENIRQ_CODE

/** @def TOUCH_USE_AVERAGING_CODE
 * If this defined is set the averaging option is available.
 */
// undefine this to save progmem if you not using averaging
#define TOUCH_USE_AVERAGING_CODE

/** @def TOUCH_USE_USER_CALIBRATION
 * If this defined is set the member function getUserCalibration() is available.
 */
// undefine this to save progmem if you don't use getUserCalibration() anymore
#define TOUCH_USE_USER_CALIBRATION

/** @def TOUCH_USE_SIMPE_TARGET
 * If this defined is set the function getUserCalibration() show simple target.
 */
// define this to save progmem, simple target wont write outside dispay coordinates (undefine it looks nicer, bud can core on some dispay drivers)
#define TOUCH_USE_SIMPE_TARGET

/** @def TOUCH_USE_DIFFERENTIAL_MEASURE
 * If this defined is set the 'Differential Measure' mode is used. (SER/DFR low)
 * When undefined 'Single Ended Measure' mode is active. (SER/DFR high) 
 */
#define TOUCH_USE_DIFFERENTIAL_MEASURE

/** @def BASIC_FONT_SUPPORT
 * If this defined is set the tft driver lacks setTextFont() and drawString().
 */
//#define BASIC_FONT_SUPPORT

/** @def TOUCH_SERIAL_DEBUG
 * If this defined is set the library show info to Serial.
 */
// define this to see additional output on Serial
//#define TOUCH_SERIAL_DEBUG

/** @def TOUCH_SERIAL_CONVERSATION_TIME
 * If this defined is set the library show time of one conversation in microsecond to Serial.
 * When TOUCH_SERIAL_DEBUG_FETCH is defined you got additional time from the print call!
 * When defined, getUserCalibration() won't work.
 */
// define this to see converation time on Serial for fetching raw values
//#define TOUCH_SERIAL_CONVERSATION_TIME

/** @def TOUCH_SERIAL_DEBUG_FETCH
 * If this defined is set the library show ctrl command of invalid read measure to Serial.
 */
// define this to see if X, X, Z1 or Z2 measure is out of range when not touched
//#define TOUCH_SERIAL_DEBUG_FETCH

#ifdef TEENSYDUINO
// color used by TFT_eTouch
#define TFT_BLACK ILI9341_BLACK
#define TFT_BLUE  ILI9341_BLUE
#define TFT_GREEN ILI9341_GREEN
#define TFT_RED   ILI9341_RED
#define TFT_WHITE ILI9341_WHITE

// missing setTextFont, drawString
#ifndef BASIC_FONT_SUPPORT
#define BASIC_FONT_SUPPORT
#endif
#endif


#ifdef DOXYGEN
// we set all for documentation
#define TOUCH_USE_PENIRQ_CODE
#define TOUCH_USE_AVERAGING_CODE
#define TOUCH_USE_USER_CALIBRATION
#define TOUCH_USE_SIMPE_TARGET
#define TOUCH_USE_DIFFERENTIAL_MEASURE
#define BASIC_FONT_SUPPORT
#define TOUCH_SERIAL_DEBUG
#define TOUCH_SERIAL_CONVERSATION_TIME
#define TOUCH_SERIAL_DEBUG_FETCH
#endif

#endif // TFT_E_TOUCH_USER_H
