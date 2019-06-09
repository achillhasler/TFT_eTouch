/**
  Sketch to see raw touch values.

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
//#define MINMAX
#define WITH_DISPLAY // some Displays make touch signal noisy
//#define SECOND_SPI_PORT

//------------------------------------------------------------------------------------------
#ifdef SECOND_SPI_PORT
#define TFT_ETOUCH_SCK 14
#define TFT_ETOUCH_MISO 12
#define TFT_ETOUCH_MOSI 13

SPIClass hSPI(HSPI);
#endif
//------------------------------------------------------------------------------------------

#ifdef WITH_DISPLAY
# ifdef _ADAFRUIT_ILI9341H_
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
TFT_eTouch<Adafruit_ILI9341> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ);

# elif defined (_ILI9341_t3H_)
ILI9341_t3 tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
TFT_eTouch<ILI9341_t3> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ);

# elif defined (_TFT_eSPIH_)
TFT_eSPI tft;
#   ifdef SECOND_SPI_PORT
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, hSPI);
#   else
#     ifdef ESP32_PARALLEL
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, SPI);
#     else
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, TFT_eSPI::getSPIinstance());
#     endif
#   endif
# else
#   error definition missing in TFT_eTouchUser.h
# endif
#else

# ifdef SECOND_SPI_PORT
TFT_eTouchBase touch(TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, hSPI);
# else
TFT_eTouchBase touch(TFT_ETOUCH_CS, TFT_ETOUCH_PIRQ, SPI);
# endif

#endif

void setup() {
  Serial.begin(115200);
  delay(2000);

#ifdef SECOND_SPI_PORT
//  hSPI.begin(); // ESP32 default SCK: 14, MISO: 12, MOSI: 13, SS :15.
  hSPI.begin(TFT_ETOUCH_SCK, TFT_ETOUCH_MISO, TFT_ETOUCH_MOSI, TFT_ETOUCH_CS);
#endif
#ifdef ESP32_PARALLEL
  SPI.begin();
#endif
#ifdef WITH_DISPLAY
  tft.begin();
  touch.init();
#else
#ifdef TEENSYDUINO
  SPI.setMOSI(TFT_MOSI);
  SPI.setMISO(TFT_MISO);
  SPI.setSCK(TFT_SCLK);
#else
  // raw test on ESP32 support only default SPI bus (MISO, MOSI & SCLK from pins_arduino.h)
#endif
  touch.init(true);
#endif
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
//  touch.setRXPlate(1000);
//  touch.setRZThreshold(1000*3);

#ifdef WITH_DISPLAY
  tft.setRotation(TFT_ROTATION);
  delay(1);
  tft.fillScreen(TFT_BLACK);
#ifdef BASIC_FONT_SUPPORT
  const char* str;
  str = "Touch raw test!";
  int16_t len = strlen(str) * 6;
  tft.setCursor((tft.width() - len)/2, tft.height()/2);
  tft.print(str);
#else
  tft.drawCentreString("Touch raw test!", tft.width()/2, tft.height()/2, 2);
#endif
#endif
}

void loop(void) {
  static uint32_t last_update = 0;
  if (last_update + touch.getMeasureWait() > millis()) return;
  last_update = millis();
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
//  delay(touch.getMeasureWait() + 1); // wait a little bit longer, for getting new measure values
}
