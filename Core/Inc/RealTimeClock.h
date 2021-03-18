
#ifndef RealTimeClock_h
#define RealTimeClock_h

#include "stm32f4xx_hal.h"
#include "Timezone.h"


class RealTimeClock
{
private:
	RTC_HandleTypeDef* 	_hrtc;
	Timezone*			_timezone;

	time_t toTimeT( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate );
	void fromTimeT( time_t t, RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate );

public: 
	RealTimeClock(RTC_HandleTypeDef* _hrtc, Timezone* timezone );

	void getDateTime( RTC_TimeTypeDef*, RTC_DateTypeDef* );
	void setDateTime( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate );


};

#endif
