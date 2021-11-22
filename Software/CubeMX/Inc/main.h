/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
	uint16_t Value;			// Time for Reload Timer in ms 0 = STOP
	uint16_t Tick;				// Counting Timer
	struct{
		uint8_t OverCount	:5;	// Count OverFlow
		uint8_t Reload		:1;	// Auto Reload
		uint8_t Over 		:1;	// OverFlow
		uint8_t Run 		:1;	// RunTimer
	}Control;
}TIMER;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void InitTimers(void);
void TimerAction(void);			/* pracuje s Timerem */
void ReadFromEprom(void);
void SaveToEprom(void);
void RestoreFromEprom(void);

#define TRACE_SWO

#ifdef TRACE_SWO
uint16_t _write(uint16_t file, char *ptr, uint16_t len);
#endif

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define VM_Pin GPIO_PIN_0
#define VM_GPIO_Port GPIOA
#define IM_Pin GPIO_PIN_1
#define IM_GPIO_Port GPIOA
#define BPUSH_Pin GPIO_PIN_2
#define BPUSH_GPIO_Port GPIOA
#define BPUSH_EXTI_IRQn EXTI2_IRQn
#define BPULL_Pin GPIO_PIN_3
#define BPULL_GPIO_Port GPIOA
#define BPULL_EXTI_IRQn EXTI3_IRQn
#define BCFG_Pin GPIO_PIN_4
#define BCFG_GPIO_Port GPIOA
#define BCFG_EXTI_IRQn EXTI4_IRQn
#define SYSCLK_Pin GPIO_PIN_5
#define SYSCLK_GPIO_Port GPIOA
#define PB_Pin GPIO_PIN_6
#define PB_GPIO_Port GPIOA
#define PA_Pin GPIO_PIN_7
#define PA_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_1
#define BEEP_GPIO_Port GPIOB
#define TEST1_Pin GPIO_PIN_8
#define TEST1_GPIO_Port GPIOA
#define TEST_Pin GPIO_PIN_9
#define TEST_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_6
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_7
#define I2C_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
