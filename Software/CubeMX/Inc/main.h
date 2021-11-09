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
	uint16_t Time;				// Doba Timeru v ms 0 = stoji
	uint16_t Tick;				// Pocitani Timeru
	struct{
		uint8_t OverCount	:6;	// Count Over Timer
		uint8_t Reload		:1;	// Auto Reload
		uint8_t Over 		:1;	// Preteceni
	}Flag;
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VM_Pin GPIO_PIN_0
#define VM_GPIO_Port GPIOA
#define IM_Pin GPIO_PIN_1
#define IM_GPIO_Port GPIOA
#define BPUSH_Pin GPIO_PIN_2
#define BPUSH_GPIO_Port GPIOA
#define BPULL_Pin GPIO_PIN_3
#define BPULL_GPIO_Port GPIOA
#define BCFG_Pin GPIO_PIN_4
#define BCFG_GPIO_Port GPIOA
#define SYSCLK_Pin GPIO_PIN_5
#define SYSCLK_GPIO_Port GPIOA
#define PB_Pin GPIO_PIN_6
#define PB_GPIO_Port GPIOA
#define PA_Pin GPIO_PIN_7
#define PA_GPIO_Port GPIOA
#define TEST1_Pin GPIO_PIN_8
#define TEST1_GPIO_Port GPIOA
#define TEST_Pin GPIO_PIN_9
#define TEST_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
