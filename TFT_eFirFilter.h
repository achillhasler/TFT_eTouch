#ifndef TFT_E_FIR_FILTER_H
#define TFT_E_FIR_FILTER_H

//
//  TFT_eFirFilter.h
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

/** 
  * @brief  fir filter
  * @param  N number of coefficients, must be even
  * @param  T used data type
  */
template <uint16_t N = 12, class T = uint16_t>
class FirFilter
{
public:
  FirFilter(uint8_t filter = 1)
  : divisor_(0)
  , filled_(0)
  {
    calc_q(filter);
  }

  void calc_q(uint8_t filter)
  {
    // calculate q for Hamming window
    // w[n] = 0.54 - 0.46·cos(2·pi·n/N)
    // w[n] = 0.5·[1-cos(2·pi·n/N)] Hanning window
    // w[n] = 0.42 - 0.5·cos(2·pi·n/N) + 0.08·cos(4·pi·n/N) Blackmann window
    for (uint16_t i = 0; i < N/2+1; i++) {
      uint8_t qi;
      switch (filter) {
      case 1: // Hamming window
        qi = (uint8_t)((0.54 - 0.46 * cos(2 * PI * i / N)) * 255);
        break;
      case 2: // Hanning window
        qi = (uint8_t)((0.5 * (1 - cos(2 * PI * i / N))) * 255);
        break;
      case 3: // Blackmann window
        qi = (uint8_t)((0.42 - 0.5 * cos(2 * PI * i / N) + 0.08 * cos(4 * PI * i / N)) * 255);
        break;
      default:
        qi = 0;
      }
      divisor_ += 2*qi;
      q_[i] = qi;
#ifdef TOUCH_SERIAL_DEBUG
      if (Serial) {
        Serial.printf("q[%i]: %i\n", i, q_[i]);
      }
#endif
    }
    divisor_ -= 255; // q[N/2+1] only once
    if (N <= 8)
      divisor_ = divisor_ * 1000/1024; // some adjustment N=6
    else if (N <= 14)
      divisor_ = divisor_ * 1011/1024; // some adjustment N=12
    else if (N <= 26)
      divisor_ = divisor_ * 1017/1024; // some adjustment N=20
    else
      divisor_ = divisor_ * 1020/1024; // some adjustment N=40

#ifdef TOUCH_SERIAL_DEBUG
      if (Serial) {
        Serial.printf("divisor: %i\n", divisor_);
      }
#endif

    reset();
  }

  T next(T val)
  {
    T ret = 0;
    buffer_[act_] = val;
    if (filled_ < N) {
      filled_++;
      if (filled_ < N) {
        act_++;
        return ret;
      }
#ifdef TOUCH_SERIAL_DEBUG
      else {
        if (Serial) {
          Serial.print("q: ");
          for (uint16_t i = 0; i < N/2; i++) {
            Serial.printf("%i, ", q_[i]);
          }
          Serial.println(q_[N/2]);
        }
      }
#endif
    }
    uint32_t sum = 0;
    for (uint16_t i = 0; i < N; i++) {
      uint16_t pos_q = i < N/2+1 ? i : N - i;
      uint16_t pos_b = (N + act_ - i) % N;
      sum += buffer_[pos_b] * q_[pos_q];
    }
    ret = sum / divisor_;
    act_++;
    if (act_ >= N) act_ = 0;

    return ret;
  }

  inline void reset()
  {
    filled_ = 0;
    act_ = 0;
  }

  inline uint16_t size() const
  {
    return N;
  }

  inline uint16_t filled() const
  {
    return filled_;
  }

private:
  T buffer_[N];
  uint8_t q_[N/2+1];
  uint32_t divisor_;
  uint16_t filled_;
  uint16_t act_;
};

#endif // TFT_E_FIR_FILTER_H
