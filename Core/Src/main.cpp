/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "max7219.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "zstrtok.h"
#include "RealTimeClock.h"
#include "Bounce2.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#define BUF_SIZE 128

char buf[BUF_SIZE];
char rbuf[BUF_SIZE] = {0};
char gpsdata[BUF_SIZE];

volatile bool gpsfound = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// This array holds the settings for switching the segments off/on to display digits
// from 0-9. Note that a '0' indicates a lit segment as the display is common anode.
uint8_t digits[10] = {
		0b00000011,
		0b10011111,
		0b00100101,
		0b00001101,
		0b10011001,
		0b01001001,
		0b01000001,
		0b00011111,
		0b00000001,
		0b00001001
};

typedef struct POS {
	int 	degrees;
	float  	minutes;
	char	direction;
} Pos;

typedef struct GPSINFO {
	int 	hours;
	int 	mins;
	int 	secs;
	int 	day;
	int 	month;
	int 	year;
	bool 	valid;
	Pos 	latitude;
	Pos 	longitude;
	int		timeUpdated;
	int		timeLastDisciplined;
	bool	disciplined;
} GPSInfo;

TimeChangeRule EDT = { "EDT", Second, Sun, Mar, 2, -240 };    //Daylight time = UTC - 4 hours
TimeChangeRule EST = { "EST", First, Sun, Nov, 2, -300 };     //Standard time = UTC - 5 hours
Timezone Eastern(EDT, EST);
TimeChangeRule CDT = { "CDT", Second, Sun, Mar, 2, -300 };    //Daylight time = UTC - 5 hours
TimeChangeRule CST = { "CST", First, Sun, Nov, 2, -360 };     //Standard time = UTC - 6 hours
Timezone Central(CDT, CST);
TimeChangeRule MDT = { "MDT", Second, Sun, Mar, 2, -360 };    //Daylight time = UTC - 6 hours
TimeChangeRule MST = { "MST", First, Sun, Nov, 2, -420 };     //Standard time = UTC - 7 hours
Timezone Mountain(MDT, MST);
TimeChangeRule PDT = { "PDT", Second, Sun, Mar, 2, -420 };    //Daylight time = UTC - 7 hours
TimeChangeRule PST = { "PST", First, Sun, Nov, 2, -480 };     //Standard time = UTC - 8 hours
Timezone Pacific(PDT, PST);

enum ClockModes {
    DisplayTime, DisplayDate, DisplayAlarm
};

enum DateDisplayModes {
    DDMMYY, MMDDYY
};

enum TimeDisplayModes {
    HR24, HR12
};

Bounce	mode;
Bounce	up;
Bounce	down;

ClockModes			clockMode = DisplayTime;
DateDisplayModes	dateDisplayMode = DDMMYY;
TimeDisplayModes	timeDisplayMode = HR24;

volatile GPSInfo	gpsInfo = {0};

void printUART( const char* format, ...);
void parseGPS( char *g, volatile GPSInfo* gpsInfo );
void parseGGA( char *g, volatile GPSInfo* gpsInfo );
void parseGSA( char *g, volatile GPSInfo* gpsInfo );
void parseRMC( char *g, volatile GPSInfo* gpsInfo );

void processMode();
void processUp();
void processDown();

void displayTimeSPI( RealTimeClock* rtc, MAX7219* max7219, volatile GPSInfo* gpsInfo );
void displayDateSPI( RealTimeClock* rtc, MAX7219* max7219, volatile GPSInfo* gpsInfo );
void disciplineClock( RealTimeClock* rtc, volatile GPSInfo* );
void setClock( RealTimeClock* rtc );

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  memset( rbuf, 0, BUF_SIZE );

  printUART( "\r\nStarting GPS\r\n\r\n" );
  HAL_UART_Receive_IT(&huart6, (uint8_t *)buf, 1);

  mode.attach( MODE_GPIO_Port, MODE_Pin );
  mode.interval(20);
  up.attach( UP_GPIO_Port, UP_Pin );
  up.interval(20);
  down.attach( DOWN_GPIO_Port, DOWN_Pin );
  down.interval(20);

  MAX7219 max7219( &hspi1, GPIOB, GPIO_PIN_4 );
  max7219.Begin();
  max7219.MAX7219_SetBrightness( '\07');

  int timeLastDisplay = HAL_GetTick();
  RealTimeClock rtc( &hrtc, &Central );

  //setClock( &rtc );

  while (1)
  {

	  if ( mode.update() )
		  processMode();

	  if ( up.update() && up.read() == 0 )
		  processUp();

	  if ( down.update() && down.read() == 0 )
		  processDown();

	  disciplineClock( &rtc, &gpsInfo );

	  if ( gpsfound )
		  parseGPS( gpsdata, &gpsInfo );

	  if (clockMode == DisplayTime )
	  {
		  if ( HAL_GetTick() - timeLastDisplay > 100 )
		  {
			  timeLastDisplay = HAL_GetTick();
			  displayTimeSPI( &rtc, &max7219, &gpsInfo );
		  }
	  }

	  else if ( clockMode == DisplayDate )
	  {
		  if ( HAL_GetTick() - timeLastDisplay > 100 )
		  {
			  timeLastDisplay = HAL_GetTick();
			  displayDateSPI( &rtc, &max7219, &gpsInfo );
		  }

	  }

	  //printUART( "%d %02d:%02d:%02d\r\n", HAL_GetTick(), sTime.Hours, sTime.Minutes, sTime.Seconds);
	  //HAL_Delay(2);
  }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


/* USER CODE BEGIN 4 */

void processMode()
{
	if ( mode.read() == 0 )
	{
		if ( clockMode == DisplayDate )
			clockMode = DisplayTime;
		else if ( clockMode == DisplayTime )
			clockMode = DisplayDate;
	}
}


void processUp()
{
	if ( clockMode == DisplayDate )
	{
		if ( dateDisplayMode == DDMMYY )
			dateDisplayMode = MMDDYY;
		else
			dateDisplayMode = DDMMYY;
	}

	else if ( clockMode == DisplayTime )
	{
		if ( timeDisplayMode == HR12 )
			timeDisplayMode = HR24;
		else
			timeDisplayMode = HR12;
	}
}

void processDown()
{
	if ( clockMode == DisplayDate )
		processUp();
	else if ( clockMode == DisplayTime )
		processUp();
}


void setClock( RealTimeClock* rtc )
{
	  RTC_TimeTypeDef sTime = {0};
	  RTC_DateTypeDef sDate = {0};

	  sTime.Hours = 12;
	  sTime.Minutes = 59;
	  sTime.Seconds = 50;

	  sDate.Month = 3;
	  sDate.Date = 17;
	  sDate.Year = 21;

	  rtc->setDateTime(&sTime, &sDate);
}

//
// Discipline the real time clock from GPS. If the clock is not disciplined then
// do this straight away otherwise perform only once per minute. Not sure whether this is
// entirely necessary but it seems wrong to update the RTC constantly
//
// The GPS info has to be valid (determined by GPGSA returning a valid 2D or 3D fix
// and not stale - determined by making sure the time the gpsInfo was update is no
// more than 200 msecs ago
//
void disciplineClock( RealTimeClock* rtc, volatile GPSInfo* gpsInfo )
{
	  if ( !gpsInfo->disciplined ||
		   HAL_GetTick() - gpsInfo->timeLastDisciplined > 3600000 /* 600000 */ ) {

		  if ( gpsInfo->valid && HAL_GetTick() - gpsInfo->timeUpdated < 200 ) {

			  RTC_TimeTypeDef sTime = {0};
			  RTC_DateTypeDef sDate = {0};
			  rtc->getDateTime( &sTime, &sDate );

			  printUART( "Current Date/Time: %02d/%02d/%02d %02d:%02d:%02d\r\n",
					  sDate.Date, sDate.Month, sDate.Year,
					  sTime.Hours, sTime.Minutes, sTime.Seconds
			  );

			  sTime.Hours = gpsInfo->hours;
			  sTime.Minutes = gpsInfo->mins;
			  sTime.Seconds = gpsInfo->secs;
			  sDate.Date = gpsInfo->day;
			  sDate.Month = gpsInfo->month;
			  sDate.Year = gpsInfo->year;

			  rtc->setDateTime( &sTime, &sDate );

			  gpsInfo->disciplined = true;
			  gpsInfo->timeLastDisciplined = HAL_GetTick();
			  gpsInfo->valid = false;

			  printUART( "Date/Time Discplined from GPS: %02d/%02d/%02d %02d:%02d:%02d\r\n\r\n",
					  sDate.Date, sDate.Month, sDate.Year,
					  sTime.Hours, sTime.Minutes, sTime.Seconds
			  );

		  }

	  }
}


void displayTimeSPI( RealTimeClock* rtc, MAX7219* max7219, volatile GPSInfo* gpsInfo )
{

	// Get the time from the real time clock and display it. Note that
	// according to HAL RTC documentation you have to call  RTC_GetDate after RTC_GetTime
	// otherwise the structs aren't populated correctly. See note below from the HAL RTC driver

	/*
	  * @note You must call HAL_RTC_GetDate() after HAL_RTC_GetTime() to unlock the values
	  *        in the higher-order calendar shadow registers to ensure consistency between the time and date values.
	  *        Reading RTC current time locks the values in calendar shadow registers until current date is read.
	*/

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	rtc->getDateTime( &sTime, &sDate );

	char tbuf[20];

	if ( timeDisplayMode == HR24 )
		sprintf( tbuf, "%02d-%02d-%02d",
				sTime.Hours, sTime.Minutes, sTime.Seconds);
	else
		sprintf( tbuf, "%2d.%02d.%02d %c",
				sTime.Hours % 12, sTime.Minutes, sTime.Seconds,
				sTime.Hours >= 12 ? 'P' : 'A' );

	max7219->DisplayText( tbuf, JUSTIFY_RIGHT );

}

void displayDateSPI( RealTimeClock* rtc, MAX7219* max7219, volatile GPSInfo* gpsInfo )
{

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	rtc->getDateTime( &sTime, &sDate );

	char tbuf[20];

	if ( dateDisplayMode == DDMMYY )
		sprintf( tbuf, "%02d-%02d-%02d", sDate.Date, sDate.Month, sDate.Year);
	else
		sprintf( tbuf, "%02d-%02d-%02d", sDate.Month, sDate.Date, sDate.Year);

	max7219->DisplayText( tbuf, JUSTIFY_RIGHT );

}


/*
 Simple "printf" that goes to the UART. Note that if you need to print
 floats then you need to include "-u _printf_float" in the Linker>Miscellaneous>Tool Settings
 */

void printUART( const char* format, ...)
{
	char buf[512];

	  va_list args;
	  va_start(args, format);
	  vsprintf(buf, format, args);
	  va_end(args);

	  HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 1000 );

}


/*
 *
 *

Sample GPS Messages

GPS at Startup

$GPGGA,235959.262,,,,,0,0,,,M,,M,,*4F
$GPGLL,,,,,235959.262,V,N*7D
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPGSV,1,1,00*79
$GPRMC,235959.262,V,,,,,0.00,0.00,050180,,,N*46
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32


GPS with Lock

$GPGGA,203831.000,3333.2945,N,09744.7542,W,1,6,2.75,163.8,M,-23.8,M,,*6B
$GPGLL,3333.2945,N,09744.7542,W,203831.000,A,A*40
$GPGSA,A,3,27,30,14,28,07,08,,,,,,,2.90,2.75,0.91*07
$GPGSV,3,1,12,07,76,040,21,30,51,322,22,08,50,050,20,14,38,278,22*74
$GPGSV,3,2,12,28,35,273,32,09,34,189,18,21,27,107,,01,18,135,*78
$GPGSV,3,3,12,17,14,208,,27,13,040,17,13,06,324,,04,06,165,*77
$GPRMC,203831.000,A,3333.2945,N,09744.7542,W,0.98,192.52,170221,,,A*7C
$GPVTG,192.52,T,,M,0.98,N,1.81,K,A*39

 *
 */
void parseGPS( char *g, volatile GPSInfo* gpsInfo )
{
    //printUART( "%s\n", g );

	/*
    if ( strncmp( "$GPGGA", g, 6 ) == 0 ) {
    	parseGGA( g, gpsInfo );
    }

    else if ( strncmp( "$GPGSA", g, 6 ) == 0 ) {
    	parseGSA( g, gpsInfo );
    }
    */
    if ( strncmp( "$GPRMC", g, 6 ) == 0 ) {
    	parseRMC( g, gpsInfo );
    }

	gpsfound = false;
}

/*

Parse the GPGSA message to determine whether a fix is available

         1 = No Fix, 2 = 2D Fix, 3 = 3D Fix
         ---------- ----------- ------------
$GPGSA,A,3,27,30,14,28,07,08,,,,,,,2.90,2.75,0.91*07

*/

void parseGSA( char *g, volatile GPSInfo *gpsInfo )
{
	char *token;

    //printUART( "Found GPGSA\r\n", g );

	token = zstrtok(g, ",");

	if ( token == NULL )
		return;

	char *mode = zstrtok(NULL, ",");

	if ( mode == NULL )
		return;

	char *fix = zstrtok(NULL, ",");

	if ( fix == NULL )
		return;

	if ( *fix == '2' || *fix == '3') {
		gpsInfo->valid = true;
		gpsInfo->timeUpdated = HAL_GetTick();
	}
	else {
		gpsInfo->valid = false;
	}

}


/*

Parse the GPGGA message and extract the UTC time, latitude and longitude

       UTC Time   Latitude    Longitude
       ---------- ----------- ------------
$GPGGA,203831.000,3333.2945,N,09744.7542,W,1,6,2.75,163.8,M,-23.8,M,,*6B

*/

void parseGGA( char *g, volatile GPSInfo *gpsInfo )
{
	char *token;

    //printUART( "Found GPGGA\r\n", g );

	token = zstrtok(g, ",");

	if ( token == NULL )
		return;

	char *utctime = zstrtok(NULL, ",");

	if ( utctime == NULL )
		return;

	int tim = atoi( utctime );

	gpsInfo->hours = tim / 10000;
	gpsInfo->mins = ( tim - gpsInfo->hours * 10000 ) / 100;
	gpsInfo->secs = tim % 100;

	//printUART( "Time: [%d] %02d:%02d:%02d\r\n", tim, gpsInfo->hours, gpsInfo->mins, gpsInfo->secs );

	//char *lat = zstrtok(NULL, ",");
}


/*

Parse the GPRMC message and extract the UTC time and date, latitude and longitude

       UTC Time     Latitude    Longitude                Date
       ----------   ----------- ------------             ------
$GPRMC,203831.000,A,3333.2945,N,09744.7542,W,0.98,192.52,170221,,,A*7C
$GPRMC,230114.00,A,3300.30830,N,09711.75273,W,0.738,,190321,,,A*61

*/

void parseRMC( char *g, volatile GPSInfo *gpsInfo )
{
	char *token;
	char save[256];

	strcpy( save, g );
    //printUART( "Found GPRMC: %s\r\n", g );

	token = zstrtok(g, ",");

	if ( token == NULL )
		return;

	char *utctime = zstrtok(NULL, ",");

	if ( utctime == NULL )
		return;

	int tim = atoi( utctime );

	gpsInfo->hours = tim / 10000;
	gpsInfo->mins = ( tim - gpsInfo->hours * 10000 ) / 100;
	gpsInfo->secs = tim % 100;

	char *mode = zstrtok(NULL, ",");
	if ( mode == NULL )
		return;

	if ( *mode == 'A' ) {
		gpsInfo->valid = true;
		gpsInfo->timeUpdated = HAL_GetTick();
	}
	else {
		gpsInfo->valid = false;
	}

	char *latdeg = zstrtok(NULL, ",");
	if ( latdeg == NULL )
		return;

	char *lat = zstrtok(NULL, ",");
	if ( lat == NULL )
		return;

	char *longdeg = zstrtok(NULL, ",");
	if ( longdeg == NULL )
		return;

	char *longdir = zstrtok(NULL, ",");
	if ( longdir == NULL )
		return;

	char *speed = zstrtok(NULL, ",");
	if ( speed == NULL )
		return;

	char *track = zstrtok(NULL, ",");
	if ( track == NULL )
		return;

	char *date = zstrtok(NULL, ",");
	if ( date == NULL )
		return;

	int dt = atoi( date );

	gpsInfo->day = dt / 10000;
	gpsInfo->month = ( dt - gpsInfo->day * 10000 ) / 100;
	gpsInfo->year = dt % 100;

	if ( gpsInfo->year != 21 )
	    printUART( "******* Error GPRMC: %s\r\n", save );

	//printUART( "Date: [%d] %02d:%02d:%02d\r\n", dt, gpsInfo->day, gpsInfo->month, gpsInfo->year );

	//char *lat = zstrtok(NULL, ",");
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	  if(huart->Instance==USART6)
	  {
	    static unsigned char uLength = 0;
	    if(buf[0] == '\n')
	    {
	      uLength  = 0;
	      if ( strncmp( rbuf, "$GP", 3 ) == 0 )
	      {
	    	  	  gpsfound = true;
	    		  memcpy( gpsdata, rbuf, BUF_SIZE);
	      }
	      memset( rbuf, 0, BUF_SIZE );
	    }
	    else
	    {
	      rbuf[uLength++] = buf[0];
	    }

	    HAL_UART_Receive_IT(&huart6, (uint8_t *)buf, 1);

	  }

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  if ( GPIO_Pin == GPIO_PIN_12 ) {
		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef sDate;
		HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN);

		//printUART( "%02d %04d %04d\r\n", sTime.Seconds, sTime.SubSeconds, sTime.SecondFraction);

  }

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
