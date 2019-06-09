//
//  TFT_eTouchGesture.cpp
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

#include <TFT_eTouchGesture.h>

void TFT_eTouchGesture::FilteredMeasure::operator=(const TFT_eTouchBase::Measure& raw)
{
  x = raw.x >> 4;
  y = raw.y >> 4;
  z1 = raw.z1 >> 4;
  z2 = raw.z2 >> 4;
  if (raw.rz == 0xffff) rz = 255;
  else if (raw.rz > (254<<4)) rz = 254;
  else rz = raw.rz >> 4;
  ms = millis();
}

TFT_eTouchGesture::TFT_eTouchGesture(uint16_t size)
: data_(0)
, size_(size)
, next_(0)
, used_(0)
{
  data_ = new FilteredMeasure[size];
}

TFT_eTouchGesture::~TFT_eTouchGesture()
{
 delete data_;
}

void TFT_eTouchGesture::set(const TFT_eTouchBase::Measure& raw)
{
  data_[next_++] = raw;
  if (next_ >= size_) next_ = 0;
  if (used_ < size_) used_++;
}

void TFT_eTouchGesture::reset()
{
  next_ = 0;
  used_ = 0;
}
#define compare_ind(i1, i2) \
if (data_[i1].x < data_[i2].x) xneg=false; \
if (data_[i1].x > data_[i2].x) xpos=false; \
if (data_[i1].y < data_[i2].y) yneg=false; \
if (data_[i1].y > data_[i2].y) ypos=false

TFT_eTouchGesture::Action TFT_eTouchGesture::get(int16_t& angle)
{
  TFT_eTouchGesture::Action ret = none;
  if (used_ < size_) return ret;
  uint16_t ind = (size_ + next_ - 1) % size_;
//  int32_t dist = sqrt(pow((data_[ind].x - data_[next_].x), 2) + pow((data_[ind].y - data_[next_].y), 2));
//  float dist = std::hypotf(data_[ind].x - data_[next_].x, data_[ind].y - data_[next_].y);
  float dist = ::hypotf(data_[ind].x - data_[next_].x, data_[ind].y - data_[next_].y);
  
  bool xneg=true, xpos=true, yneg=true, ypos=true;
  for (int i = next_; i < size_ - 1; i++) {
    compare_ind(i, i+1);
  }
  if (next_ != 0) { 
    compare_ind(size_-1, 0);
  }
  for (int i = 0; i < next_ - 2; i++) {
    compare_ind(i, i+1);
  }
  return ret;
}
#undef compare_ind

/*
    none,
    stay,
    move,
    wipe,
    zoom_in,
    zoom_out

TFT_eTouchBase::Measure* data_;

uint16_t size_;
uint16_t next_;
uint16_t used_;
*/
