
#include "RealTimeClock.h"

RealTimeClock::RealTimeClock( RTC_HandleTypeDef* hrtc, Timezone* timezone )
{
	_hrtc = hrtc;
	_timezone = timezone;
}

// Get the time from the real time clock. Note that
// according to HAL RTC documentation you have to call  RTC_GetDate after RTC_GetTime
// otherwise the structs aren't populated correctly. See note below from the HAL RTC driver

/*
  * @note You must call HAL_RTC_GetDate() after HAL_RTC_GetTime() to unlock the values
  *        in the higher-order calendar shadow registers to ensure consistency between the time and date values.
  *        Reading RTC current time locks the values in calendar shadow registers until current date is read.
*/

void RealTimeClock::getDateTime( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate )
{
	HAL_RTC_GetTime( _hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate( _hrtc, sDate, RTC_FORMAT_BIN);
}

//
// UTC time passed in and Converted to local time
//
void RealTimeClock::setDateTime( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate )
{

	time_t utc = toTimeT( sTime, sDate );
	time_t local = _timezone->toLocal(utc);
	fromTimeT( local, sTime, sDate );

	HAL_RTC_SetTime( _hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate( _hrtc, sDate, RTC_FORMAT_BIN);

}

time_t RealTimeClock::toTimeT( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate )
{
	  int yr = sDate->Year+30;

	  tmElements_t tm;
	  tm.Hour = sTime->Hours;
	  tm.Minute = sTime->Minutes;
	  tm.Second = sTime->Seconds;
	  tm.Day = sDate->Date;
	  tm.Month = sDate->Month;
	  tm.Year = yr;
	  return makeTime(tm);

}

void RealTimeClock::fromTimeT( time_t t, RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate )
{
	  tmElements_t tm;
	  breakTime(t, tm);

	  sTime->Hours = tm.Hour;
	  sTime->Minutes = tm.Minute;
	  sTime->Seconds = tm.Second;
	  sDate->Date = tm.Day;
	  sDate->Month = tm.Month;
	  sDate->Year = tm.Year - 2000;

}
