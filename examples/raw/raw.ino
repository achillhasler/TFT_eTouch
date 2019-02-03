/**
  Sketch to see raw touch values.

  The sketch has been tested on the ESP32 with compatible ADS7846 chip.
*/

#include <SPI.h>
#include <TFT_eTouch.h>

//------------------------------------------------------------------------------------------

#define TFT_ETOUCH_CS 16

#define TFT_ROTATION 3
//#define MINMAX
//define WITH_DISPLAY

//------------------------------------------------------------------------------------------

#ifdef WITH_DISPLAY
TFT_eSPI tft;
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS);
#else
TFT_eTouchBase touch(TFT_ETOUCH_CS);
#endif

void setup() {
  Serial.begin(115200);

#ifdef WITH_DISPLAY
  tft.init();
#endif
  touch.init();
//  touch.setMeasure(false, true, true, false, 1, false, false, false); // Differential mode fastest (each axis read only once, may work)
//  touch.setMeasure(true, true, true, false, 1, false, false, false); // Differential mode faster (take second z bud first x, y. may work)
//  touch.setMeasure(true, false, false, true, 14, true, true, false); // slowest (drop first and additional 16 measures. averaging 14 without min max value)
//  touch.setMeasure(); // defaults (accurate all axis, start with x)
  touch.setMeasure(true, true, true, true); // z with local min, acurate x,y (my favorite)
//  touch.setMeasure(false, false, true, false, 3, false, false, false); // constructor defaults (take third measure, start with z axis)
//  touch.setMeasure(false, false, false, false, 2, false, false, true); // Single-Ended mode (fastest) use 2'end measure
//  touch.setValidRawRange(10, 4085);
  touch.setRXPlate(333);
  touch.setRZThreshold(1000);

//  TFT_eTouch<TFT_eSPI>::Callibation calData = { 265, 3790, 264, 3850, 2 };
//  touch.setCalibration(calData);

#ifdef WITH_DISPLAY
  tft.setRotation(TFT_ROTATION);
  delay(1);
  tft.fillScreen(TFT_BLACK);
  tft.drawCentreString("Touch raw test!", tft.width()/2, tft.height()/2, 2);
#endif
}

void loop(void) {
  uint16_t x = 0, y = 0, z1 = 0, z2 = 0, rz = 0; // To store the touch coordinates
  if (!touch.getRaw(x, y, z1, z2, rz)) return;
  
#ifdef MINMAX
  static uint16_t count = 0;
  static uint16_t x_min = 0xffff, y_min = 0xffff, z1_min = 0xffff, z2_min = 0xffff, rz_min = 0xffff;
  static uint16_t x_max = 0, y_max = 0, z1_max = 0, z2_max = 0, rz_max = 0; // To store the touch coordinates
  if (x < x_min) x_min = x; if (x > x_max) x_max = x;
  if (y < y_min) y_min = y; if (y > y_max) y_max = y;
  if (z1 < z1_min) z1_min = z1; if (z1 > z1_max) z1_max = z1;
  if (z2 < z2_min) z2_min = z2; if (z2 > z2_max) z2_max = z2;
  if (rz < rz_min) rz_min = rz; if (rz > rz_max) rz_max = rz;
  if (count++ > 100) {
    count = 0;
    Serial.print("x (min, max),y (min, max),z1 (min, max),z2 (min, max),rz (min, max) = (");
    Serial.print(x_min);
    Serial.print(",");
    Serial.print(x_max);
    Serial.print("),(");
    Serial.print(y_min);
    Serial.print(",");
    Serial.print(y_max);
    Serial.print("),(");
    Serial.print(z1_min);
    Serial.print(",");
    Serial.print(z1_max);
    Serial.print("),(");
    Serial.print(z2_min);
    Serial.print(",");
    Serial.print(z2_max);
    Serial.print("),(");
    Serial.print(rz_min);
    Serial.print(",");
    Serial.print(rz_max);
    Serial.println(")");
  }
#else
  Serial.print("x,y,z1,z2,rz = ");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.print(z1);
  Serial.print(",");
  Serial.print(z2);
  Serial.print(",");
  Serial.println(rz);
#endif
  delay(touch.getMeasureWait() + 1); // wait a little bit longer, for getting new measure values
}
