#ifndef TFT_E_TOUCH_USER_H
#define TFT_E_TOUCH_USER_H

//
//  TFT_eTouchUser.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at http://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

// include used TFT driver
#include <TFT_eSPI.h>      // Hardware-specific TFT library

// undefine this to save progmem if you not have penirq or not using it
#define TOUCH_USE_PENIRQ_CODE

// define this to see additional output on Serial
//#define TFT_TOUCH_SERIAL_DEBUG

// define this to see if X, X, Z1 or Z2 measure is out of range when not touched
//#define TFT_TOUCH_SERIAL_DEBUG_FETCH

#endif // TFT_E_TOUCH_USER_H
