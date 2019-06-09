/**
  Sketch to generate the setup() calibration values, these are reported
  to the Serial Monitor.

  The sketch has been tested with compatible ADS7846 chip on:
  - ESP32 (TFT_eSPI, Adafruit_ILI9341)
  - ESP8322 (TFT_eSPI)
  - Tensy 3.1 (ILI9341_t3)
  - Arduino Nano (Adafruit_ILI9341)
*/

#include <SPI.h>
#include <TFT_eTouch.h>

//------------------------------------------------------------------------------------------

#define TFT_ROTATION 1

#define DEFAULT_CALIBRATION
#define MAX_CALIBRATION 1  /* 0 .. 4  set it to 0 when callData ok */

//------------------------------------------------------------------------------------------

# ifdef _ADAFRUIT_ILI9341H_
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
TFT_eTouch<Adafruit_ILI9341> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ);

# elif defined (_ILI9341_t3H_)
ILI9341_t3 tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
TFT_eTouch<ILI9341_t3> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ);

# elif defined (_TFT_eSPIH_)
TFT_eSPI tft;
#   ifdef ESP32_PARALLEL
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, SPI);
#   else
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, TFT_eSPI::getSPIinstance());
#   endif
# else
#   error definition missing in TFT_eTouchUser.h
# endif


void start_screen()
{
  // Clear the screen
  tft.fillScreen(TFT_BLACK);

#ifdef BASIC_FONT_SUPPORT
  tft.setTextSize(1);
  const char* str;
  str = "Touch screen!";
  int16_t len = strlen(str) * 6;
  tft.setCursor((tft.width() - len)/2, tft.height()/2 - 20);
  tft.print(str);

  str = "R: rotate, C: calibrate";
  len = strlen(str) * 6;
  tft.setCursor((tft.width()-len)/2 , tft.height()/2);
  tft.print(str);

  str = "open serial monitor to see calibration";
  len = strlen(str) * 6;
  tft.setCursor((tft.width()-len)/2 , tft.height()/2 + 20);
  tft.print(str);
#else
  tft.drawCentreString("Touch screen!", tft.width()/2, tft.height()/2 - 20, 2);
  tft.drawCentreString("R: rotate, C: calibrate", tft.width()/2, tft.height()/2, 2);
  tft.drawCentreString("open serial monitor to see calibration", tft.width()/2, tft.height()/2 + 20, 2);
#endif
  tft.setCursor(0, 0);

#ifdef BASIC_FONT_SUPPORT
  tft.setTextSize(2);
  #define CHAR_XSIZE_PIX (2 * 6)
#else
  tft.setTextFont(2);
  tft.setTextSize(1);
  #define CHAR_XSIZE_PIX 10
#endif
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.print("R");
  tft.setCursor(tft.width() - CHAR_XSIZE_PIX, 0);
  tft.print("C");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
#ifdef BASIC_FONT_SUPPORT
  tft.setTextSize(1);
#endif
}

void show_cursor(int16_t x, int16_t y)
{
  // this is not a real cursor, it does not store the background under the cursor
  static bool visible = false;
  static int16_t vx = 10000, vy = 10000, vz = 10000;

  const uint8_t size = 8, hole = 3;
  uint16_t rz = touch.getRZ();
  bool same_pos = false;
  if (vx == x && vy == y) {
    if (vz == rz) return;
    same_pos = true;
  }
  if (visible && !same_pos) {
    tft.drawFastHLine(vx - size + 1, vy, size - hole, TFT_BLACK);
    tft.drawFastHLine(vx + hole, vy, size - hole, TFT_BLACK);
    tft.drawFastVLine(vx, vy - size + 1, size - hole, TFT_BLACK);
    tft.drawFastVLine(vx, vy + hole, size - hole, TFT_BLACK);  
    tft.drawPixel(vx, vy, TFT_BLACK);  
  }
  if (x < 0 || y < 0) {
    visible = false;  // coordinate invalid, hide cursor
    return;
  }
  tft.setCursor(50, 50);
  tft.print(x);
  tft.print(",");
  tft.print(y);
  tft.print(",");
  tft.print(rz);
  tft.println("    ");
  
  if (rz > touch.getRXPlate()*174/100)  tft.println("Pen                 ");
  else if (rz > touch.getRXPlate()*66/100) tft.println("Finger              ");
  else if (rz > touch.getRXPlate()*15/100) tft.println("2 Finger            ");
  else tft.println("+ 2 Finger        ");
/*
  if (rz > touch.getRZThreshold()*58/100)  tft.println("Pen                 ");
  else if (rz >= touch.getRZThreshold()*22/100) tft.println("Finger              ");
  else if (rz > touch.getRZThreshold()*5/100) tft.println("2 Finger            ");
  else tft.println("more then 2 Finger");
*/
  if (!same_pos) {
    tft.drawFastHLine(x - size + 1, y, size - hole, TFT_WHITE);
    tft.drawFastHLine(x + hole, y, size - hole, TFT_WHITE);
    tft.drawFastVLine(x, y - size + 1, size - hole, TFT_WHITE);
    tft.drawFastVLine(x, y + hole, size - hole, TFT_WHITE);
    tft.drawPixel(x, y, TFT_WHITE); 
  }
  vx = x; vy = y; vz = rz;
  visible = true;
}

//------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(2000);

#ifdef ESP32_PARALLEL
  SPI.begin();
#endif
  tft.begin();
  touch.init();
  while (!Serial) ; // wait for Arduino Serial Monitor
//  touch.setMeasure(0, false, true, false, 3); // constructor defaults (take third measure, start with z axis)
//  touch.setAveraging(false, false); // constructor defaults (without averaging)

//  touch.setMeasure(0, true, true, false, 1); // Differential mode fastest (each axis read only once, may work)
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)

//  touch.setMeasure(1, true, true, false, 1); // Differential mode faster (take second z bud first x, y. may work)
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)

//  touch.setMeasure(10, false, false, true, 14); // slowest (drop 10 and additional 16 measures. averaging 14 with min max value removed)
//  touch.setAveraging(true, true);

  // when noisy
  //touch.setMeasure(1, false, true, true, 3); // z first, drop 1 and additional 5 measures. averaging 3 with min max value removed
  //touch.setAveraging(true, true);
  //touch.setAcurateDistance(100); // or even higher for getUserCalibration (default 10)
  // or
  touch.setAcurateDistance(80); // and define TOUCH_FILTER_TYPE in TFT_eTouchUser.h

  // untouched: 35 us touched: 95 us (good choice for starting)
  touch.setMeasure(0, false, true, false, 2); // z first, take 2'th z,x,y
/* if you get output like that when calling getUserCalibration, you have a noisy touch signal
acurate on point[4, 4](233, 371) dx: 287 dy: 27 > 10 nok
acurate on point[4, 4](283, 365) dx: 192 dy: 17 > 10 nok
acurate on point[4, 4](349, 399) dx: 192 dy: 16 > 10 nok
acurate on point[4, 4](334, 400) dx: 204 dy: 11 > 10 nok
*/


//  touch.setMeasure(); // defaults (accurate all axis, start with x)
//  touch.setMeasure(1, true, true, true); // z with local min, acurate x,y

//  touch.setValidRawRange(10, 4085);

  touch.setRXPlate(1000/3);
  touch.setRZThreshold(1000);

#ifndef DEFAULT_CALIBRATION
  TFT_eTouchBase::Calibation calibation = { 265, 3790, 264, 3850, 2 }; // x and y axes have same direction on touch & display
//  TFT_eTouchBase::Calibation calibation = { 3790, 265, 3850, 264, 0 }; // same as above
//  TFT_eTouchBase::Calibation calibation = { 272, 3749, 3894, 341, 0 }; // y axes have oposite direction

  // Calibrate the touch screen and retrieve the scaling factors
  for (int rot = 0; rot < MAX_CALIBRATION; rot++) {
    // Set the rotation before we calibrate
    tft.setRotation(TFT_ROTATION + rot % 4);
    touch.getUserCalibration(calibation, 4);
  }
  touch.setCalibration(calibation);
#endif
  tft.setRotation(TFT_ROTATION);
  delay(1);
  start_screen();
}

//------------------------------------------------------------------------------------------

void loop(void) {
  static uint32_t last_update = 0;
  if (last_update + touch.getMeasureWait() > millis()) return;
  last_update = millis();
  
  static bool is_touched = false;
  int16_t x, y; // screen coordinates
  if (touch.getXY(x, y)) {
    if (!is_touched) { // we have pen down!
      is_touched = true;
    }
    if (y < 10) {
      if (x < 10) {
        show_cursor(-1, -1); // hide cursor
        uint8_t rot = tft.getRotation();
        rot = (rot + 1) % 4;
        tft.setRotation(rot);
        start_screen();
        return;
      }
      if (x > tft.width() - 10) {
        TFT_eTouchBase::Calibation calibation;
        show_cursor(-1, -1); // hide cursor
        touch.getUserCalibration(calibation, 4);
        touch.setCalibration(calibation);
        start_screen();
        return;
      }
    }
    show_cursor(x, y);
  }
  else {
    if (is_touched) { // we have pen up!
      is_touched = false;
      show_cursor(-1, -1); // hide cursor
      touch.reset();
    }
  }

}
