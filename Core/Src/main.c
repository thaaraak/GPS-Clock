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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */



  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_0, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_3, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_6, GPIO_PIN_RESET );
  HAL_GPIO_WritePin( GPIOA, GPIO_PIN_7, GPIO_PIN_RESET );

  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_2, GPIO_PIN_SET );
  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_1, GPIO_PIN_SET );
  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_SET );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_SET );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_SET );

  int currentIdx = 0;
  int MAX_IDX = 6;

  int displayNumber = 123456;
  int tim = HAL_GetTick();
  memset( rbuf, 0, BUF_SIZE );

  HAL_UART_Transmit(&huart1, "Hello", 5, 1000 );
  HAL_UART_Receive_IT(&huart6, (uint8_t *)buf, 1);

  while (1)
  {

	  if ( gpsfound )
		  parseGPS();

	  /*
	  int digit = display( displayNumber, currentIdx );
	  currentIdx++;

	  if ( currentIdx >= MAX_IDX )
		  currentIdx = 0;

	  HAL_Delay(1);

	  if ( HAL_GetTick() - tim > 1000 )
	  {
		  tim = HAL_GetTick();
		  displayNumber++;
	  }


	  /*
	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_2, GPIO_PIN_RESET );

	  for ( int i = 0 ; i < 10 ; i++ ) {
		  GPIOA->ODR = ( GPIOA->ODR & 0xff00 ) | digits[i];
		  HAL_Delay(500);
	  }

	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_2, GPIO_PIN_SET );

	  /*
	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_1, GPIO_PIN_RESET );
	  HAL_Delay(2);
	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_1, GPIO_PIN_SET );

	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_RESET );
	  HAL_Delay(2);
	  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );

	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_RESET );
	  HAL_Delay(2);
	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_SET );

	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_RESET );
	  HAL_Delay(2);
	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_SET );


	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_RESET );
	  HAL_Delay(2);
	  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_SET );

	  /*
	  flashPin( GPIOA, GPIO_PIN_0);
	  flashPin( GPIOA, GPIO_PIN_1);
	  flashPin( GPIOA, GPIO_PIN_2);
	  flashPin( GPIOA, GPIO_PIN_3);
	  flashPin( GPIOA, GPIO_PIN_4);
	  flashPin( GPIOA, GPIO_PIN_5);
	  flashPin( GPIOA, GPIO_PIN_6);
	  flashPin( GPIOA, GPIO_PIN_7);

*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


int display( int num, int idx )
{
	int val = pow( 10, idx+1 );
	int digit = ( num % val ) / (val/10);

	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;

	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_2, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_1, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_0, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_SET );

	if ( idx < 3 )
		GPIOx = GPIOC;
	else
		GPIOx = GPIOB;

	if ( idx == 5 )
		GPIO_Pin = GPIO_PIN_2;
	else if ( idx == 4 )
		GPIO_Pin = GPIO_PIN_1;
	else if ( idx == 3 )
		GPIO_Pin = GPIO_PIN_0;
	else if ( idx == 2 )
		GPIO_Pin = GPIO_PIN_15;
	else if ( idx == 1 )
		GPIO_Pin = GPIO_PIN_14;
	else if ( idx == 0 )
		GPIO_Pin = GPIO_PIN_13;

	  HAL_GPIO_WritePin( GPIOx, GPIO_Pin, GPIO_PIN_RESET );

	  GPIOA->ODR = ( GPIOA->ODR & 0xff00 ) | digits[digit];


	return digit;

}


void flashPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
	  HAL_GPIO_WritePin( GPIOx, GPIO_Pin, GPIO_PIN_RESET );
	  HAL_Delay(20);
	  HAL_GPIO_WritePin( GPIOx, GPIO_Pin, GPIO_PIN_SET );

}

void printUART( const char* format, ...)
{
	char buf[512];

	  va_list args;
	  va_start(args, format);
	  vsprintf(buf, format, args);
	  va_end(args);

	  HAL_UART_Transmit(&huart1, buf, strlen(buf), 1000 );

}

void parseGPS()
{
    char *saveptr, *token;

    printUART( "%s\n", gpsdata );

    token = strtok_r(gpsdata, ",", &saveptr);
    while ( token != NULL )
        token = strtok_r(NULL, ",", &saveptr);

	gpsfound = false;
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