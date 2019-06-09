#ifndef TFT_E_TOUCH_BASE_INL
#define TFT_E_TOUCH_BASE_INL

//
//  TFT_eTouchBase.inl
//
//  (C) Copyright Achill Hasler 2019.
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file at https://www.boost.org/LICENSE_1_0.txt
//
//
//  See TFT_eTouch/docs/html/index.html for documentation.
//

// public inline members
uint16_t TFT_eTouchBase::getRZ() const
{
  return raw_.rz;
}

void TFT_eTouchBase::setCalibration(const Calibation& data)
{
  calibation_ = data;
}

TFT_eTouchBase::Calibation& TFT_eTouchBase::calibration()
{
  return calibation_;
}

void TFT_eTouchBase::setMeasure(uint8_t drop_first, bool z_once, bool z_first, bool z_local_min, uint8_t count)
{
  drop_first_measures_ = drop_first;
  z_once_measure_ = z_once;
  z_first_measure_ = z_first;
  z_local_min_measure_ = z_local_min; 
  
  count_measure_ = count;
}

#ifdef TOUCH_USE_AVERAGING_CODE
void TFT_eTouchBase::setAveraging(bool averaging, bool ignore_min_max)
{
  averaging_measure_ = averaging; 
  ignore_min_max_measure_ = ignore_min_max;
}
#endif // end TOUCH_USE_AVERAGING_CODE

void TFT_eTouchBase::setValidRawRange(uint16_t min, uint16_t max)
{
  raw_valid_min_ = min; // raw measure minimum value from x, y, z1 and z2 (otherwise it is not touched)
  raw_valid_max_ = max; // raw measure maximum value
}


void TFT_eTouchBase::setMeasureWait(uint16_t ms)
{
  measure_wait_ms_ = ms;
}
uint16_t TFT_eTouchBase::getMeasureWait() const
{
  return measure_wait_ms_;
}


void TFT_eTouchBase::setRXPlate(uint16_t ohm)
{
  rx_plate_ = ohm;
}
uint16_t TFT_eTouchBase::getRXPlate() const
{
  return rx_plate_;
}

void TFT_eTouchBase::setRZThreshold(uint16_t ohm)
{
  rz_threshold_ = ohm;
}
uint16_t TFT_eTouchBase::getRZThreshold() const
{
  return rz_threshold_;
}

void TFT_eTouchBase::reset()
{
#ifdef TOUCH_FILTER_TYPE
# ifdef TOUCH_X_FILTER 
  x_filter_->reset();
# endif
# ifdef TOUCH_Y_FILTER 
  y_filter_->reset();
# endif
# ifdef TOUCH_Z_FILTER
  z1_filter_->reset();
  z2_filter_->reset();
# endif
#endif
}


bool TFT_eTouchBase::valid()
{
  return raw_.rz < rz_threshold_;
}

#ifdef TOUCH_USE_USER_CALIBRATION
void TFT_eTouchBase::setAcurateDistance(uint16_t raw_difference)
{
  acurate_difference_ = raw_difference;
}
uint16_t TFT_eTouchBase::getAcurateDistance() const
{
  return acurate_difference_;
}
#endif // TOUCH_USE_USER_CALIBRATION

// private inline members

bool TFT_eTouchBase::is_touched()
{
  return raw_.rz != 0xffff;
}

bool TFT_eTouchBase::in_range(uint16_t measure)
{
  return (measure >= raw_valid_min_) && (measure <= raw_valid_max_);
}

void TFT_eTouchBase::spi_start()
{
	spi_.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	digitalWrite(cs_, LOW);
}

void TFT_eTouchBase::spi_end()
{
	digitalWrite(cs_, HIGH);
	spi_.endTransaction();
}

#endif // TFT_E_TOUCH_BASE_INL
