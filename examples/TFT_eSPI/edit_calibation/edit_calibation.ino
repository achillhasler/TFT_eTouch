/**
  Sketch to generate the setup() calibration values, these are reported
  to the Serial Monitor.

  The sketch has been tested on the ESP32 with compatible ADS7846 chip and TFT_eSPI driver.
  define TEST_RAW_INTERFACE in TMenu.h for raw test.
*/

#include <SPI.h>
#include <TFT_eTouch.h>

#include "TMenu.h"

//------------------------------------------------------------------------------------------
#define TFT_ROTATION 3

//------------------------------------------------------------------------------------------

TFT_eSPI tft;
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, 0xff, TFT_eSPI::getSPIinstance());

TMenu menue(touch);

//------------------------------------------------------------------------------------------
#define CALIBRATION_FILE "/TFT_eTouch.cal"

void setup() {
  Serial.begin(115200);
  delay(1000);

  tft.begin();
  touch.init();

  while (!Serial) ; // wait for Arduino Serial Monitor

  //SPIFFS.remove(CALIBRATION_FILE);

  if (!touch.readCalibration(CALIBRATION_FILE)) {
#ifdef TOUCH_USE_USER_CALIBRATION
    TFT_eTouchBase::Calibation cal;
    touch.getUserCalibration(cal, 4);
    touch.setCalibration(cal);
#else
    Serial.printf("Calibration not readed %s take default configuration\n", CALIBRATION_FILE);
    touch.calibration() = { 265, 3790, 264, 3850, 2 };
#endif
  }

  // untouched: 35 us touched: 136 us
//  touch.setMeasure(0, false, true, false, 3); // constructor defaults (take third measure, start with z axis)
//  touch.setAveraging(false, false); // constructor defaults (without averaging)

  // untouched: 35 us touched: 56 us
//  touch.setMeasure(0, true, true, false, 1); // Differential mode fastest (each axis read only once, may work)
#ifdef TOUCH_USE_USER_CALIBRATION
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)
#endif
  
  // untouched: 35 us touched: 77 us
//  touch.setMeasure(1, true, true, false, 1); // Differential mode faster (take second z bud first x, y. may work)
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)

  // untouched: 35 us touched: d 190 max 600 us
//  touch.setMeasure(1, true, true, true); // z with local min, acurate x,y, 35 us tot d 200 max 600
  
  // untouched: 35 us touched: d 220 max 600 us
//  touch.setMeasure(0, false, true); // z first, acurate z,x,y

  // untouched: 35 us touched: d 190 max 600 us
//  touch.setMeasure(2, true, true, false); // z first 3end, acurate x,y

  // untouched: 35 us touched: 95 us
//  touch.setMeasure(1, true, true, false, 2); // z first, take 2'th z,x,y

  // untouched: 35 us touched: 95 us
//  touch.setMeasure(0, false, true, false, 2); // z first, take 2'th z,x,y

  // untouched: 35 us touched: 1302 us
//  touch.setMeasure(10, false, true, true, 14); // slowest (drop 10(..255) and additional 16 measures. averaging 14 with min max value)
//  touch.setAveraging(true, true);

  // untouched: 35 us touched: 334 us
//  touch.setMeasure(3, false, true, false, 3); // drop 3 averaging 3 (+2 min max)
//  touch.setAveraging(true, true);

  touch.setRXPlate(1000);
  touch.setRZThreshold(1000*3);
  touch.setMeasureWait(10);

  tft.setRotation(TFT_ROTATION);
  menue.init(); // after rotation
}

//------------------------------------------------------------------------------------------

void loop(void) {
  static bool was_touched = false;
  static uint32_t last_touch = 0;

  if (last_touch + touch.getMeasureWait() > millis()) return;
  last_touch = millis();

#ifdef TEST_RAW_INTERFACE
  TFT_eTouchBase::Measure raw;
  if (touch.getRaw(raw)) {
    menue.update(raw);
#else
  int16_t x, y; // screen coordinates
  if (touch.getXY(x, y)) {
    menue.update(x, y, touch.getRZ());
#endif
    if (!was_touched) {
      was_touched = true;
      Serial.println("pen down");
      // we have pen down
      menue.pen_down();
    }
    menue.refresh();
//    Serial.println("pen activ");
  }
  else {
    if (was_touched) {
      was_touched = false;
      Serial.print("pen up, ");
      // we have pen up
      switch (menue.pen_up()) {
        case changed:
          Serial.println("sig changed");
        break;
        case calibrate: {
#ifdef TOUCH_USE_USER_CALIBRATION
          Serial.println("sig calibrate");
          TFT_eTouchBase::Calibation cal;
          touch.getUserCalibration(cal, 4);
          touch.setCalibration(cal);
          menue.init();
#else
          Serial.println("sig calibrate disabled");
#endif
        }
        break;
        case store:
          Serial.println("sig store");
          touch.writeCalibration(CALIBRATION_FILE);
        break;
        case none:
        default:
          Serial.println("sig none");
        break;
      }
      menue.refresh();
    }
  }
}
