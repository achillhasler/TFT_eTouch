/**
  Sketch to generate the setup() calibration values, these are reported
  to the Serial Monitor.

  The sketch has been tested on the Tennsy 3.1 with xpt2046 chip and ILI9341_t3 driver.
*/

#include <SPI.h>
#include <TFT_eTouch.h>

//------------------------------------------------------------------------------------------

#ifndef TEENSYDUINO
#error This is a Tennsy demo
#endif

#define TFT_ROTATION 3
#define TFT_DC      20  // Data Command control pin
#define TFT_CS      21  // Chip select pin
#define TFT_RST      2  // on SoftwareReset use 255, and connect pin to 3.3V
#define TFT_MOSI     7  // MasterOut SlaveIn pin (DOUT)
#define TFT_SCLK    14  // SPI clock (SCK)
#define TFT_MISO     8  // MasterIn SlaveOut pin (DIN)
#define TFT_BL       3  // Backlight pin must have PWM for analogWrite if set


// #define DEFAULT_CALIBRATION
#define MAX_CALIBRATION 1  /* 0 .. 4  set it to 0 when callData ok */

//------------------------------------------------------------------------------------------

ILI9341_t3 tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
TFT_eTouch<ILI9341_t3> touch(tft, TFT_ETOUCH_CS);


void start_screen()
{
  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  char* str;
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

  tft.setCursor(0, 0);

  tft.setTextSize(2);

  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.print("R");
  tft.setCursor(tft.width() - 12, 0);
  tft.print("C");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
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
  
  if (rz > touch_.getRXPlate()*174/100)  tft().println("Pen                 ");
  else if (rz >= touch_.getRXPlate()*66/100) tft().println("Finger              ");
  else if (rz > touch_.getRXPlate()*15/100) tft().println("2 Finger            ");
  else tft().println("+ 2 Finger        ");
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
  delay(1000);

  tft.begin();
  touch.init();
//  touch.setMeasure(0, false, true, false, 3); // constructor defaults (take third measure, start with z axis)
//  touch.setAveraging(false, false); // constructor defaults (without averaging)

//  touch.setMeasure(0, true, true, false, 1); // Differential mode fastest (each axis read only once, may work)
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)

//  touch.setMeasure(1, true, true, false, 1); // Differential mode faster (take second z bud first x, y. may work)
//  touch.setAcurateDistance(25); // in this mode acurate distance must be higher for getUserCalibration (default 10)

//  touch.setMeasure(10, false, false, true, 14); // slowest (drop 10 and additional 16 measures. averaging 14 with min max value)
//  touch.setAveraging(true, true);

//  touch.setMeasure(); // defaults (accurate all axis, start with x)
//  touch.setMeasure(1, true, true, true); // z with local min, acurate x,y

//  touch.setValidRawRange(10, 4085);
//  touch.setRXPlate(1000);
//  touch.setRZThreshold(3*1000);

#ifndef DEFAULT_CALIBRATION
  TFT_eTouchBase::Calibation calibation = { 265, 3790, 264, 3850, 2 }; // x and y axes have same direction on touch & display
//  TFT_eTouchBase::Calibation calibation = { 3790, 265, 3850, 264, 0 }; // same as above
//  TFT_eTouchBase::Calibation calibation = { 265, 3790, 3850, 264, 2 }; // y axes have oposite direction

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
  int16_t x, y; // screen coordinates
  if (touch.getXY(x, y)) {
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
    delay(touch.getMeasureWait() + 1); // wait a little bit longer, for getting new measure values
  }
  else show_cursor(-1, -1); // hide cursor
}
