// The Game of Life, also known simply as Life, is a cellular automaton
// devised by the British mathematician John Horton Conway in 1970.
// https://en.wikipedia.org/wiki/Conway's_Game_of_Life

// touching the corner
// ul: reset this sample
// ur: goto next sample
// ll: speedup animation
// lr: slowdown animation
// touch outside corner will set a cell

// this sample use touch implemention from TFT_eSPI when TOUCH_CS is defined in User_Setup.h otherwiese TFT_eTouch is used.

#include <FS.h>
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library
#ifndef TOUCH_CS
#include <TFT_eTouch.h>
#endif

TFT_eSPI tft;         // Invoke custom library

#ifndef TOUCH_CS
TFT_eTouch<TFT_eSPI> touch(tft, TFT_ETOUCH_CS, 0xff, TFT_eSPI::getSPIinstance());
#endif

#define CELL_SIZE 2     // cell in pixel can be 1
#define ROTATION 3

#ifdef TOUCH_CS
#define CALIBRATION_FILE "/TouchCalibData3"
#else
#define CALIBRATION_FILE "/TFT_eTouch.cal"
#endif

#define BYTES(bits) (((bits) + 7) / 8)

class LifeData {
  uint16_t x_grid_;
  uint16_t y_grid_;
  uint8_t* data_; // [XBYTES * GRIDY];
public:
  LifeData(TFT_eSPI& tft, uint8_t cell_size)
  : x_grid_(tft.width()/cell_size)
  , y_grid_(tft.height()/cell_size)
  , data_(new uint8_t[BYTES(x_grid_) * y_grid_])
  {}

  ~LifeData()
  {
    if (data_) delete data_;
  }

  inline void clear()
  {
    memset(data_, 0, BYTES(x_grid_) * y_grid_);
  }

  inline void operator=(const LifeData& d)
  {
    memcpy(data_, d.data_, BYTES(x_grid_) * y_grid_);
  }
  
  void fill(uint8_t border);
  void pattern(uint8_t type, uint16_t x_off, uint16_t y_off);

  inline void set_pos(int16_t x, int16_t y, bool set)
  {
    if ((x >= x_grid_) || (y >= y_grid_)) return;
    uint8_t x_byte = x / 8, x_bit = x % 8;
    if (set) data_[x_byte + y * BYTES(x_grid_)] |= _BV(x_bit);
    else     data_[x_byte + y * BYTES(x_grid_)] &= ~_BV(x_bit);
  }

  inline bool get_pos(int16_t x, int16_t y) const
  {
    if ((x >= x_grid_) || (y >= y_grid_)) return false;
    uint8_t x_byte = x / 8, x_bit = x % 8;
    return (data_[x_byte + y * BYTES(x_grid_)] & _BV(x_bit)) != 0;
  }
  
  uint8_t neighbors(int16_t x, int16_t y) const;
  void print() const;
};


class Life {
  TFT_eSPI& tft_;
  uint8_t cell_size_;
  LifeData grid_, nextgrid_;
  uint8_t generation_delay_, border_;

  void draw();
  void compute();
  uint8_t touched();
public:
  Life(TFT_eSPI& tft_, uint8_t cell_size = 3, uint8_t border = 255)
  : tft_(tft)
  , cell_size_(cell_size)
  , grid_(tft_, cell_size)
  , nextgrid_(tft_, cell_size)
  , generation_delay_(0)
  , border_(border)
  { }

  void begin();
  void step();
};

void LifeData::pattern(uint8_t type, uint16_t x_off, uint16_t y_off)
{
  switch(type) {
  case 0: // Lightweight spaceship going right (LWSS)
    set_pos(1 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(2 + x_off, 4 + y_off, true);
    set_pos(3 + x_off, 4 + y_off, true);
    set_pos(4 + x_off, 4 + y_off, true);
    set_pos(5 + x_off, 4 + y_off, true);
    set_pos(5 + x_off, 3 + y_off, true);
    set_pos(5 + x_off, 2 + y_off, true);
    set_pos(4 + x_off, 1 + y_off, true);
  break;
  case 1: // Lightweight spaceship going left (LWSS)
    set_pos(6 - 1 + x_off, 1 + y_off, true);
    set_pos(6 - 1 + x_off, 3 + y_off, true);
    set_pos(6 - 2 + x_off, 4 + y_off, true);
    set_pos(6 - 3 + x_off, 4 + y_off, true);
    set_pos(6 - 4 + x_off, 4 + y_off, true);
    set_pos(6 - 5 + x_off, 4 + y_off, true);
    set_pos(6 - 5 + x_off, 3 + y_off, true);
    set_pos(6 - 5 + x_off, 2 + y_off, true);
    set_pos(6 - 4 + x_off, 1 + y_off, true);
  break;
  case 2: // Middleweight spaceship going right (MWSS)
    set_pos(7 - 4 + x_off, 1 + y_off, true);
    set_pos(7 - 2 + x_off, 2 + y_off, true);
    set_pos(7 - 6 + x_off, 2 + y_off, true);
    set_pos(7 - 1 + x_off, 3 + y_off, true);
    set_pos(7 - 1 + x_off, 4 + y_off, true);
    set_pos(7 - 6 + x_off, 4 + y_off, true);
    set_pos(7 - 1 + x_off, 5 + y_off, true);
    set_pos(7 - 2 + x_off, 5 + y_off, true);
    set_pos(7 - 3 + x_off, 5 + y_off, true);
    set_pos(7 - 4 + x_off, 5 + y_off, true);
    set_pos(7 - 5 + x_off, 5 + y_off, true);
  break;
  case 3: // Middleweight spaceship going left (MWSS)  
    set_pos(4 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(6 + x_off, 2 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(1 + x_off, 4 + y_off, true);
    set_pos(6 + x_off, 4 + y_off, true);
    set_pos(1 + x_off, 5 + y_off, true);
    set_pos(2 + x_off, 5 + y_off, true);
    set_pos(3 + x_off, 5 + y_off, true);
    set_pos(4 + x_off, 5 + y_off, true);
    set_pos(5 + x_off, 5 + y_off, true);
  break;
  case 4: // Heavyweight spaceship going right (HWSS)
    y_off += 2;
    set_pos(8 - 1 + x_off, 1 + y_off, true);
    set_pos(8 - 2 + x_off, 1 + y_off, true);
    set_pos(8 - 3 + x_off, 1 + y_off, true);
    set_pos(8 - 4 + x_off, 1 + y_off, true);
    set_pos(8 - 5 + x_off, 1 + y_off, true);
    set_pos(8 - 6 + x_off, 1 + y_off, true);
    set_pos(8 - 1 + x_off, 2 + y_off, true);
    set_pos(8 - 7 + x_off, 2 + y_off, true);
    set_pos(8 - 1 + x_off, 3 + y_off, true);
    set_pos(8 - 2 + x_off, 4 + y_off, true);
    set_pos(8 - 7 + x_off, 4 + y_off, true);
    set_pos(8 - 4 + x_off, 5 + y_off, true);
    set_pos(8 - 5 + x_off, 5 + y_off, true);
  break;
  case 5: // Heavyweight spaceship going left (HWSS)
    y_off += 2;
    set_pos(1 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(4 + x_off, 1 + y_off, true);
    set_pos(5 + x_off, 1 + y_off, true);
    set_pos(6 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(7 + x_off, 2 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(2 + x_off, 4 + y_off, true);
    set_pos(7 + x_off, 4 + y_off, true);
    set_pos(4 + x_off, 5 + y_off, true);
    set_pos(5 + x_off, 5 + y_off, true);
  break;
  case 6: // Glider going down right
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(3 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 3 + y_off, true);
    set_pos(3 + x_off, 3 + y_off, true);
  break;
  case 7: // Glider going down left
    set_pos(4 - 3 + x_off, 1 + y_off, true);
    set_pos(4 - 1 + x_off, 2 + y_off, true);
    set_pos(4 - 3 + x_off, 2 + y_off, true);
    set_pos(4 - 2 + x_off, 3 + y_off, true);
    set_pos(4 - 3 + x_off, 3 + y_off, true);
  break;
  case 8: // Glider going up right
    set_pos(3 + x_off, 4 - 1 + y_off, true);
    set_pos(1 + x_off, 4 - 2 + y_off, true);
    set_pos(3 + x_off, 4 - 2 + y_off, true);
    set_pos(2 + x_off, 4 - 3 + y_off, true);
    set_pos(3 + x_off, 4 - 3 + y_off, true);
  break;
  case 9: // Glider going up left
    set_pos(4 - 3 + x_off, 4 - 1 + y_off, true);
    set_pos(4 - 1 + x_off, 4 - 2 + y_off, true);
    set_pos(4 - 3 + x_off, 4 - 2 + y_off, true);
    set_pos(4 - 2 + x_off, 4 - 3 + y_off, true);
    set_pos(4 - 3 + x_off, 4 - 3 + y_off, true);
  break;
  case 10: // Blinker
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(3 + x_off, 2 + y_off, true);
  break;
  case 11: // Toad
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(4 + x_off, 2 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(4 + x_off, 3 + y_off, true);
    set_pos(4 + x_off, 4 + y_off, true);
  break;
  case 12: // Beacon
    set_pos(1 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(3 + x_off, 3 + y_off, true);
    set_pos(4 + x_off, 3 + y_off, true);
    set_pos(3 + x_off, 4 + y_off, true);
    set_pos(4 + x_off, 4 + y_off, true);
  break;
  case 13: // Pulsar
    x_off++;
    y_off++;
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(4 + x_off, 1 + y_off, true);
    set_pos(5 + x_off, 1 + y_off, true);
    set_pos(9 + x_off, 1 + y_off, true);
    set_pos(10 + x_off, 1 + y_off, true);
    set_pos(11 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(6 + x_off, 3 + y_off, true);
    set_pos(8 + x_off, 3 + y_off, true);
    set_pos(13 + x_off, 3 + y_off, true);
    set_pos(1 + x_off, 4 + y_off, true);
    set_pos(6 + x_off, 4 + y_off, true);
    set_pos(8 + x_off, 4 + y_off, true);
    set_pos(13 + x_off, 4 + y_off, true);
    set_pos(1 + x_off, 5 + y_off, true);
    set_pos(6 + x_off, 5 + y_off, true);
    set_pos(8 + x_off, 5 + y_off, true);
    set_pos(13 + x_off, 5 + y_off, true);
    set_pos(3 + x_off, 6 + y_off, true);
    set_pos(4 + x_off, 6 + y_off, true);
    set_pos(5 + x_off, 6 + y_off, true);
    set_pos(9 + x_off, 6 + y_off, true);
    set_pos(10 + x_off, 6 + y_off, true);
    set_pos(11 + x_off, 6 + y_off, true);
    set_pos(3 + x_off, 8 + y_off, true);
    set_pos(4 + x_off, 8 + y_off, true);
    set_pos(5 + x_off, 8 + y_off, true);
    set_pos(9 + x_off, 8 + y_off, true);
    set_pos(10 + x_off, 8 + y_off, true);
    set_pos(11 + x_off, 8 + y_off, true);
    set_pos(1 + x_off, 9 + y_off, true);
    set_pos(6 + x_off, 9 + y_off, true);
    set_pos(8 + x_off, 9 + y_off, true);
    set_pos(13 + x_off, 9 + y_off, true);
    set_pos(1 + x_off, 10 + y_off, true);
    set_pos(6 + x_off, 10 + y_off, true);
    set_pos(8 + x_off, 10 + y_off, true);
    set_pos(13 + x_off, 10 + y_off, true);
    set_pos(1 + x_off, 11 + y_off, true);
    set_pos(6 + x_off, 11 + y_off, true);
    set_pos(8 + x_off, 11 + y_off, true);
    set_pos(13 + x_off, 11 + y_off, true);
    set_pos(3 + x_off, 13 + y_off, true);
    set_pos(4 + x_off, 13 + y_off, true);
    set_pos(5 + x_off, 13 + y_off, true);
    set_pos(9 + x_off, 13 + y_off, true);
    set_pos(10 + x_off, 13 + y_off, true);
    set_pos(11 + x_off, 13 + y_off, true);
  break;

  case 14: // Pentadecathlon v (period 15)
    x_off += 3;
    y_off += 3;
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(1 + x_off, 3 + y_off, true);
    set_pos(3 + x_off, 3 + y_off, true);
    set_pos(2 + x_off, 4 + y_off, true);
    set_pos(2 + x_off, 5 + y_off, true);
    set_pos(2 + x_off, 6 + y_off, true);
    set_pos(2 + x_off, 7 + y_off, true);
    set_pos(1 + x_off, 8 + y_off, true);
    set_pos(3 + x_off, 8 + y_off, true);
    set_pos(2 + x_off, 9 + y_off, true);
    set_pos(2 + x_off, 10 + y_off, true);
  break;

  case 15: // Pentadecathlon h (period 15)
    x_off += 3;
    y_off += 3;
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(8 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(4 + x_off, 2 + y_off, true);
    set_pos(5 + x_off, 2 + y_off, true);
    set_pos(6 + x_off, 2 + y_off, true);
    set_pos(7 + x_off, 2 + y_off, true);
    set_pos(9 + x_off, 2 + y_off, true);
    set_pos(10 + x_off, 2 + y_off, true);
    set_pos(3 + x_off, 3 + y_off, true);
    set_pos(8 + x_off, 3 + y_off, true);
  break;

  case 16: // The R-pentomino
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 3 + y_off, true);
  break;

  case 17: // Diehard
    set_pos(7 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 2 + y_off, true);
    set_pos(2 + x_off, 3 + y_off, true);
    set_pos(6 + x_off, 3 + y_off, true);
    set_pos(7 + x_off, 3 + y_off, true);
    set_pos(8 + x_off, 3 + y_off, true);
  break;

  case 18: // infinite Line
    y_off -= 1;
    set_pos(1 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(4 + x_off, 1 + y_off, true);
    set_pos(5 + x_off, 1 + y_off, true);
    set_pos(6 + x_off, 1 + y_off, true);
    set_pos(7 + x_off, 1 + y_off, true);
    set_pos(8 + x_off, 1 + y_off, true);
    set_pos(10 + x_off, 1 + y_off, true);
    set_pos(11 + x_off, 1 + y_off, true);
    set_pos(12 + x_off, 1 + y_off, true);
    set_pos(13 + x_off, 1 + y_off, true);
    set_pos(14 + x_off, 1 + y_off, true);
    set_pos(18 + x_off, 1 + y_off, true);
    set_pos(19 + x_off, 1 + y_off, true);
    set_pos(20 + x_off, 1 + y_off, true);
    set_pos(27 + x_off, 1 + y_off, true);
    set_pos(28 + x_off, 1 + y_off, true);
    set_pos(29 + x_off, 1 + y_off, true);
    set_pos(30 + x_off, 1 + y_off, true);
    set_pos(31 + x_off, 1 + y_off, true);
    set_pos(32 + x_off, 1 + y_off, true);
    set_pos(33 + x_off, 1 + y_off, true);
    set_pos(35 + x_off, 1 + y_off, true);
    set_pos(36 + x_off, 1 + y_off, true);
    set_pos(37 + x_off, 1 + y_off, true);
    set_pos(38 + x_off, 1 + y_off, true);
    set_pos(39 + x_off, 1 + y_off, true);
  break;

  case 19: // infinite 5x5
    set_pos(1 + x_off, 1 + y_off, true);
    set_pos(2 + x_off, 1 + y_off, true);
    set_pos(3 + x_off, 1 + y_off, true);
    set_pos(5 + x_off, 1 + y_off, true);
    set_pos(1 + x_off, 2 + y_off, true);
    set_pos(4 + x_off, 3 + y_off, true);
    set_pos(5 + x_off, 3 + y_off, true);
    set_pos(2 + x_off, 4 + y_off, true);
    set_pos(3 + x_off, 4 + y_off, true);
    set_pos(5 + x_off, 4 + y_off, true);
    set_pos(1 + x_off, 5 + y_off, true);
    set_pos(3 + x_off, 5 + y_off, true);
    set_pos(5 + x_off, 5 + y_off, true);
  break;

  default:
  break;
  }
}

void LifeData::fill(uint8_t border)
{
  int x_off = x_grid_/2;
  int y_off = y_grid_/2;
  clear();
  switch(border) {
  case 255: // The R-pentomino
    pattern(16, x_off, y_off);
  break;
  case 254: // Diehard
    pattern(17, x_off-x_off/2, y_off-y_off/2);
    pattern(17, x_off, y_off-y_off/2);
    pattern(17, x_off+x_off/2, y_off-y_off/2);

    pattern(17, x_off-x_off/2, y_off);
    pattern(17, x_off, y_off);
    pattern(17, x_off+x_off/2, y_off);

    pattern(17, x_off-x_off/2, y_off+y_off/2);
    pattern(17, x_off, y_off+y_off/2);
    pattern(17, x_off+x_off/2, y_off+y_off/2);
  break;
  case 253: //  infinite Line
    y_off -= 1;
    pattern(18, 6, y_off);
  break;
  case 252: // infinite 5x5 
    x_off = x_grid_*4/5;
    y_off += y_off/2;
    pattern(19, x_off, y_off);
  break;
  case 251: // spaceships
    pattern(0, x_off, 3);
    pattern(1, x_off, 18 + 3);
    pattern(2, x_off, y_off - 18/2);
    pattern(3, x_off, y_off + 18/2);
    pattern(4, x_off, y_grid_ - 18 - 10);
    pattern(5, x_off, y_grid_ - 10);
  break;
  case 250: // Glider  
    pattern(6, x_off, y_grid_/2);
    pattern(6, x_off + 13, y_grid_/2);
    pattern(6, x_off + 2*13, y_grid_/2);
    pattern(6, x_off + 3*13, y_grid_/2);
    pattern(6, x_off + 4*13, y_grid_/2);
    pattern(6, x_off + 5*13, y_grid_/2);
    pattern(6, x_off + 6*13, y_grid_/2);
    pattern(6, x_off + 7*13, y_grid_/2);
    pattern(7, x_off - 13, y_grid_/2);
    pattern(7, x_off - 2*13, y_grid_/2);
    pattern(7, x_off - 3*13, y_grid_/2);
    pattern(7, x_off - 4*13, y_grid_/2);
    pattern(7, x_off - 5*13, y_grid_/2);
    pattern(7, x_off - 6*13, y_grid_/2);
    pattern(7, x_off - 7*13, y_grid_/2);
    pattern(7, x_off - 8*13, y_grid_/2);
    pattern(8, x_off, y_grid_/2 - 13);
    pattern(8, x_off + 13, y_grid_/2 - 13);
    pattern(8, x_off + 2*13, y_grid_/2 - 13);
    pattern(8, x_off + 3*13, y_grid_/2 - 13);
    pattern(8, x_off + 4*13, y_grid_/2 - 13);
    pattern(8, x_off + 5*13, y_grid_/2 - 13);
    pattern(8, x_off + 6*13, y_grid_/2 - 13);
    pattern(8, x_off + 7*13, y_grid_/2 - 13);
    pattern(9, x_off - 13, y_grid_/2 - 13);
    pattern(9, x_off - 2*13, y_grid_/2 - 13);
    pattern(9, x_off - 3*13, y_grid_/2 - 13);
    pattern(9, x_off - 4*13, y_grid_/2 - 13);
    pattern(9, x_off - 5*13, y_grid_/2 - 13);
    pattern(9, x_off - 6*13, y_grid_/2 - 13);
    pattern(9, x_off - 7*13, y_grid_/2 - 13);
    pattern(9, x_off - 8*13, y_grid_/2 - 13);
  break;
  case 249: // Oscillators 
    pattern(10, x_off-20, y_off-20);
    pattern(11, x_off, y_off-20);
    pattern(12, x_off+20, y_off-20);

    pattern(13, 0, 0);
    pattern(14, x_off-10, y_off);
    pattern(15, x_off+10, y_off);
  break;
  case 248: // Random
    border = y_grid_ / 4;
  default:
    if (2 * border > y_grid_ - 40) return;
    if (2 * border > x_grid_ - 40) return;
    for (int16_t x = border; x < x_grid_-border; x++) {
      for (int16_t y = border; y < y_grid_-border; y++) {
        if (random(5) == 1) set_pos(x, y, true);
      }
    }
  }
}

void LifeData::print() const
{
  Serial.print("dim: ");
  Serial.print(x_grid_);
  Serial.print(" / ");
  Serial.println(y_grid_);
  for (int16_t x = 0; x < x_grid_; x++) {
#if 1 
    for (int16_t y = 0; y < y_grid_; y++) {
      Serial.print("grid[");
      Serial.print(x);
      Serial.print(", ");
      Serial.print(y);
      Serial.print("] ");
      Serial.print(get_pos(x, y));
      Serial.print(" ");
      Serial.println(neighbors(x,y));
    }
#else
    Serial.print("grid[");
    Serial.print(x);
    Serial.print("] ");
    
    for (int16_t y = 0; y < y_grid_; y++) {
      Serial.print("[");
      Serial.print(get_pos(x, y));
      Serial.print(" ");
      Serial.print(neighbors(x,y));
      Serial.print("]");
    }
    Serial.println("");
#endif
  }
}

uint8_t LifeData::neighbors(int16_t x, int16_t y) const
{
  uint8_t neighbors = 0;

  uint16_t xpos = (x + 1) % x_grid_;
  uint16_t xneg = (x + x_grid_ - 1) % x_grid_;

  uint16_t ypos = (y + 1) % y_grid_;
  uint16_t yneg = (y + y_grid_ - 1) % y_grid_;

  if (get_pos(xneg, y)   ) neighbors++;
  if (get_pos(xneg, yneg)) neighbors++;
  if (get_pos(x,    yneg)) neighbors++;
  if (get_pos(xpos, yneg)) neighbors++;
  if (get_pos(xpos, y)   ) neighbors++;
  if (get_pos(xpos, ypos)) neighbors++;
  if (get_pos(x,    ypos)) neighbors++;
  if (get_pos(xneg, ypos)) neighbors++;

  return neighbors;
}

void Life::draw()
{
  uint16_t color = TFT_WHITE;
  for (int16_t x = 0; x < tft_.width()/cell_size_; x++) {
    for (int16_t y = 0; y < tft_.height()/cell_size_; y++) {
      if (grid_.get_pos(x, y) != nextgrid_.get_pos(x, y)) {
        if (nextgrid_.get_pos(x, y)) color = TFT_WHITE;
        else color = TFT_BLACK;
        tft_.fillRect(cell_size_ * x, cell_size_ * y, cell_size_, cell_size_, color);
      }
    }
  }
  grid_ = nextgrid_;
}

void Life::compute()
{
  nextgrid_.clear();
  for (int16_t x = 0; x < tft_.width()/cell_size_; x++) {
    for (int16_t y = 0; y < tft_.height()/cell_size_; y++) {
      bool active = grid_.get_pos(x, y);
      uint8_t neighbors = grid_.neighbors(x, y);
      if (active) {
        if (neighbors >= 2 && neighbors <= 3) {
          // stay active
          nextgrid_.set_pos(x, y, true);
        }
      }
      else if (neighbors == 3) {
        // get active
        nextgrid_.set_pos(x, y, true);
      }
    }
  }
}

uint8_t Life::touched()
{
  const int offset = 20;
#ifdef TOUCH_CS
  uint16_t x, y;
  if (tft_.getTouch(&x, &y)) {
#else
  int16_t x, y;
  if (touch.getXY(x, y)) {
#endif
    if (x < offset) {
      if (y < offset) return 2; // left upper corner
      else if (y > tft_.height() - offset) return 4;  // left lower corner
    }
    if (x > tft_.width() - offset) {
      if (y < offset) return 5; // right upper corner
      else if (y > tft_.height() - offset) return 3;  // right lower corner
    }
    nextgrid_.set_pos(x/cell_size_, y/cell_size_, true);
    return 1;    // anywhere
  }
  return 0; // nowhere
}

void Life::begin()
{
  // Display a simple splash screen
  tft_.fillScreen(TFT_BLACK);
  tft_.setTextSize(2);
  tft_.setTextColor(TFT_WHITE);
  tft_.setCursor(35, 5);
  tft_.println(F("Arduino"));
  tft_.setCursor(35, 35);
  tft_.println(F("Conway's"));
  tft_.setCursor(35, 65);
  tft_.println(F("Game of Life"));

  tft_.setCursor(35, 95);
  tft_.println(F("Touch corners"));

  tft_.setCursor(35, 125);
  switch(border_) {
  case 255: // The R-pentomino
  tft_.println(F("The R-pentomino"));
  break;
  case 254: // Diehard
  tft_.println(F("Diehard"));
  break;
  case 253: //  infinite Line
  tft_.println(F("infinite Line"));
  break;
  case 252: // infinite 5x5 
  tft_.println(F("infinite 5x5"));
  break;
  case 251: // spaceships
  tft_.println(F("Spaceships"));
  break;
  case 250: // Glider  
  tft_.println(F("Glider"));
  break;
  case 249: // Oscillators 
  tft_.println(F("Oscillators"));
  break;
  case 248: // Random
  tft_.println(F("Random"));
  break;
  default:
  tft_.println(F("Border"));
  break;
  }
  delay(2000);

  tft_.fillScreen(TFT_BLACK);
  grid_.clear();
  nextgrid_.fill(border_);
}

void Life::step()
{
  draw();
  compute();
  switch(touched()) {
    case 0: // nowhere
      break;
    case 1: // anywhere
      break;
    case 2: // left upper corner
      begin();
      break;
    case 3: // right lower corner
      if (generation_delay_ < 255) generation_delay_++;
      break;
    case 4: // left lower corner
      if (generation_delay_ > 0) generation_delay_--;
      break;
    case 5: // right upper corner
      if (border_ >= 248) {
        if (--border_ < 248) {
          border_ = 255;
        }
        begin();
      }
      break;
    default: break;
  }
  delay(generation_delay_ * 20);
}

Life* life = 0;


void setup()
{
  Serial.begin(115200);
  
  //Set up the display
  tft.init();
  tft.setRotation(ROTATION);


#ifndef TOUCH_CS
  touch.init();

  // untouched: 35 us touched: 136 us
//  touch.setMeasure(0, false, true, false, 3); // constructor defaults (take third measure, start with z axis)
//  touch.setAveraging(false, false); // constructor defaults (without averaging)

  // untouched: 35 us touched: 56 us
//  touch.setMeasure(0, true, true, false, 1); // Differential mode fastest (each axis read only once, may work)

  // untouched: 35 us touched: 77 us
//  touch.setMeasure(1, true, true, false, 1); // Differential mode faster (take second z bud first x, y. may work)

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
//  touch.setAveraging(true, true); //  averaging on (+2 min max)
   
#endif

  tft.fillScreen(TFT_WHITE);

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(1);
  tft.println("calibration run");

  //SPIFFS.remove(CALIBRATION_FILE);
  
  // check if calibration file exists
#ifdef TOUCH_CS
  uint16_t cal_data[5];
  bool has_cal_file = false;
  
  // check file system
  if (!SPIFFS.begin()) {
    Serial.println("formating file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

  if (SPIFFS.exists(CALIBRATION_FILE)) {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)cal_data, 10) == 10) has_cal_file = true;
      if (has_cal_file) {
        Serial.printf("Calibration: x %u, %u, y %u, %u, opt %u\n", cal_data[0], cal_data[1], cal_data[2], cal_data[3], cal_data[4]);
      }
      else Serial.println("Calibration File read error");
      f.close();
    }
  }
  if (has_cal_file) {
    // calibration data valid
    tft.setTouch(cal_data);
  }
  else {
    // data not valid. recalibrate
    tft.calibrateTouch(cal_data, TFT_WHITE, TFT_RED, 15);
    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)cal_data, 10);
      f.close();
    }
  }
#else
  if (!touch.readCalibration(CALIBRATION_FILE)) {
#ifdef TOUCH_USE_USER_CALIBRATION
    TFT_eTouchBase::Calibation calibation; // = { 265, 3790, 264, 3850, 2 }; // x and y axes have same direction on touch & display
    touch.getUserCalibration(calibation, 4);
    touch.setCalibration(calibation);
    touch.writeCalibration(CALIBRATION_FILE);
#else
    Serial.printf("Calibration not readed %s take default configuration. Store a valid configuration with eTouch_edit\n", CALIBRATION_FILE);
    touch.calibration() = { 265, 3790, 264, 3850, 2 };
#endif
  }
#endif

  tft.fillScreen(TFT_BLACK);
  life = new Life(tft, CELL_SIZE);
  life->begin();
}

void loop() {
  life->step();
}
