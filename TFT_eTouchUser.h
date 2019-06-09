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
#if defined (TEENSYDUINO) // with ILI9341
# define TFT_ETOUCH_CS 15
#elif defined (ESP8266) // with ILI9341
# define TFT_ETOUCH_CS D1 // Chip select pin (T_CS)
#elif 1
# define TFT_ETOUCH_CS 3
#else
# define TFT_ETOUCH_CS SS
#endif

/** @def TFT_ETOUCH_PIRQ
 * Define the irq pin for the touch chip.
 */
#if 0
# define TFT_ETOUCH_PIRQ 6
#elif 0 //defined (ESP8266) // with ILI9341
# define TFT_ETOUCH_PIRQ D0 // is touched signal pin (T_IRQ) is high when touched
#elif 0
# define TFT_ETOUCH_PIRQ 1
#else
# define TFT_ETOUCH_PIRQ 255
#endif

// include used TFT driver
#if 0 // using Adafuit lib ILI9341
#include <Adafruit_ILI9341.h>
typedef Adafruit_ILI9341 TFT_Driver;

# define TFT_DC 4
# define TFT_CS 3
# define TFT_RST 17  // on SoftwareReset use -1, and connect pin to 3.3V

#elif defined (TEENSYDUINO) // with ILI9341
# include <ILI9341_t3.h>    // Hardware-specific TFT library
typedef ILI9341_t3 TFT_Driver;

# define TFT_DC      20  // Data Command control pin
# define TFT_CS      21  // Chip select pin
# define TFT_RST      2  // on SoftwareReset use 255, and connect pin to 3.3V
# define TFT_MOSI     7  // MasterOut SlaveIn pin (DOUT)
# define TFT_SCLK    14  // SPI clock (SCK)
# define TFT_MISO    12  // MasterIn SlaveOut pin (DIN)
# define TFT_BL       3  // Backlight pin must have PWM for analogWrite if set

#else // ESP_PLATFORM with eSPI
#include <TFT_eSPI.h>      // Hardware-specific TFT library
typedef TFT_eSPI TFT_Driver;
// manage your TFT setup in TFT_eSPI UserSetup.h

# ifdef TOUCH_CS
#error undef TOUCH_CS in TFT_eSPI UserSetup.h for using TFT_eTouch
# endif

#endif

#ifndef TFT_ETOUCH_PIRQ
# define TFT_ETOUCH_PIRQ 255
#else
# if (TFT_ETOUCH_PIRQ != 255)
// if pin is set and valid we have to use penirq code
# define TOUCH_USE_PENIRQ_CODE
# endif
#endif

/** @def TOUCH_USE_PENIRQ_CODE
 * If this define is set the penrirq code is used.
 */
// undefine this to save progmem if you not have penirq or not using it
#ifndef TOUCH_USE_PENIRQ_CODE
//#define TOUCH_USE_PENIRQ_CODE
#endif

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

/** @def TOUCH_USE_GESTURE
 * If this defined is set the gesture interface is available. (work in progress)
 */
// undefine this to save progmem if you don't use gesture interface anymore
//#define TOUCH_USE_GESTURE

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

/** @def TOUCH_DEFAULT_CALIBRATION
 * This is the used touch configuration. If it's match to your configuration, you can disable TOUCH_USE_USER_CALIBRATION.
 */
#define TOUCH_DEFAULT_CALIBRATION { 272, 3749, 3894, 341, 0 }

/** @def TOUCH_FILTER_TYPE
 * If this defined is set the touch driver filter raw data with a fir filter,
 * define additional TOUCH_X_FILTER, TOUCH_Y_FILTER, TOUCH_Z_FILTER for the value to filter
 * Window type:
 * - 1: Hamming
 * - 2: Hanning
 * - 3: Blackmann
 */
//#define TOUCH_FILTER_TYPE 1

/** @def BASIC_FONT_SUPPORT
 * If this defined is set the tft driver lacks setTextFont() and drawString().
 */
//#define BASIC_FONT_SUPPORT

/** @def TOUCH_SERIAL_DEBUG
 * If this defined is set the library show info to Serial.
 */
// define this to see additional output on Serial
#define TOUCH_SERIAL_DEBUG

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
// define this to see if X, Y, Z1 or Z2 measure is out of range when not touched
//#define TOUCH_SERIAL_DEBUG_FETCH


#ifdef TOUCH_FILTER_TYPE
#include <TFT_eFirFilter.h>
// undefine a filter or change N, N must be even (default 12), T must be uint16_t whitch is default
#define TOUCH_X_FILTER FirFilter<20>
#define TOUCH_Y_FILTER TOUCH_X_FILTER
//#define TOUCH_Z_FILTER FirFilter<>
#endif

#ifndef TOUCH_DEFAULT_CALIBRATION
#define TOUCH_DEFAULT_CALIBRATION { 300, 3700, 300, 3700, 2 }
#endif

#if defined (_ILI9341_t3H_) || defined (_ADAFRUIT_ILI9341H_)
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
// we set all for getting documentation
#define TOUCH_USE_PENIRQ_CODE
#define TOUCH_USE_AVERAGING_CODE
#define TOUCH_USE_USER_CALIBRATION
#define TOUCH_USE_SIMPE_TARGET
#define TOUCH_USE_GESTURE
#define TOUCH_USE_DIFFERENTIAL_MEASURE
#define BASIC_FONT_SUPPORT
#define TOUCH_SERIAL_DEBUG
#define TOUCH_SERIAL_CONVERSATION_TIME
#define TOUCH_SERIAL_DEBUG_FETCH
#define TOUCH_FILTER_TYPE 1
#endif

#endif // TFT_E_TOUCH_USER_H
