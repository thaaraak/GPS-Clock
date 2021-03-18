
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

	  /*
	  // seconds from 1970 till 1 jan 00:00:00 of the given year
	  seconds= yr*(SECS_PER_DAY * 365);
	  for (i = 0; i < yr; i++) {
	    if (LEAP_YEAR(i)) {
	      seconds += SECS_PER_DAY;   // add extra days for leap years
	    }
	  }

	  // add days for this year, months start from 1
	  for (i = 1; i < sDate->Month; i++) {
	    if ( (i == 2) && LEAP_YEAR(yr)) {
	      seconds += SECS_PER_DAY * 29;
	    } else {
	      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
	    }
	  }
	  seconds+= (sDate->Date) * SECS_PER_DAY;
	  seconds+= sTime->Hours * SECS_PER_HOUR;
	  seconds+= sTime->Minutes * SECS_PER_MIN;
	  seconds+= sTime->Seconds;
	  return (time_t)seconds;
	  */

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

	  /*
	  uint8_t year;
	  uint8_t month, monthLength;
	  uint32_t time;
	  unsigned long days;

	  time = (uint32_t)t;
	  sTime->Seconds = time % 60;
	  time /= 60; // now it is minutes
	  sTime->Minutes = time % 60;
	  time /= 60; // now it is hours
	  sTime->Hours = time % 24;
	  time /= 24; // now it is days
	  //tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

	  year = 0;
	  days = 0;
	  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
	    year++;
	  }
	  sDate->Year = year-30; // year is offset from 1970

	  days -= LEAP_YEAR(year) ? 366 : 365;
	  time  -= days; // now it is days in this year, starting at 0

	  days=0;
	  month=0;
	  monthLength=0;
	  for (month=0; month<12; month++) {
	    if (month==1) { // february
	      if (LEAP_YEAR(year)) {
	        monthLength=29;
	      } else {
	        monthLength=28;
	      }
	    } else {
	      monthLength = monthDays[month];
	    }

	    if (time >= monthLength) {
	      time -= monthLength;
	    } else {
	        break;
	    }
	  }
	  sDate->Month = month + 1;  // jan is month 1
	  sDate->Date = time + 1;     // day of month
	  */
}
