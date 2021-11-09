/* USER CODE BEGIN Header */
#pragma GCC diagnostic ignored "-Wunused-variable"
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include "stm32f1xx_hal_def.h"
#include "fonts.h"
#include "motor.h"
#include "measurement.h"
#include "bbb.h"
#include "utility.h"
#include "stm32f1xx_it.h"
#include "ssd1306.h"
#include "display.h"
#include "eeprom.h"
#include "tim.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BEEPDUR	50
#define CONFIGWINDOWDURATION_MS	2000


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//RTC_TimeTypeDef RtcTime;
//RTC_DateTypeDef RtcDate;

uint8_t CompareSeconds;
uint8_t CompareDate;
// for using named Timers, (Uses the named Timer in the timers field)
// when added Timer the next interrupt processing code does not
// need to change for timers
// LAST element is not in the set of active elements,
// He is only for element count in ENUM
// last Element = count active elements
// ENUM need to start from 0 and must continue sequentially 0,1,2,.....,lastElement
enum eTimers {
	OneSec = 0,
	Milis350,
	Milis40,
	CountTimers // is not in the set of active elements, only for element count in ENUM
};

volatile TIMER Timers[CountTimers]; //number of Timers is correct for this code implementation C

bbb_BtnTd *PushBtn;
bbb_BtnTd *PullBtn;
bbb_BtnTd *CfgBtn;

static uint16_t SpeedModeList[] = {10, 20, 30, 40, 50, 60, 70, 80, 99};// set speed in %
//static char* SpeedModeDesc[] = {"Min", "Low", "Med", "Hi ", "Max"};
static char* SpeedModeDesc[] = {"10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "99%"};
static uint16_t SpeedMode = 5;
static const uint16_t AdrSpeedMode = 0x1111; //virtual address to EPROM
bool SpeedModeChange = false;


static uint16_t RetractDurationList[] = {0, 100, 200, 300, 400, 500, 1000, 1500, 2000};
static char* RetractDurationDesc[] = {"Dis ", "0.1 s", "0.2 s", "0.3 s", "0.4 s", "0.5 s", "1.0 s", "1.5 s", "2.0 s"};
static uint16_t RetractDuration = 5;
static const uint16_t AdrRetractDuration = 0x2222; //virtual address to EPROM
bool RetractDurationChange = false;

static disp_CfgModeTd CfgMode = disp_CfgModeTd_None;

uint8_t RedrawRequested = 1;
volatile uint8_t I2C_DMA_Completed = 1;

extern const float		VMRESH;	//Rezistor R1 from Voltage divider									//Ohms
extern const float		VMRESL;	//Rezistor R2 from Voltage divider								//Ohms

extern const float		IMSENSR; //Measure rezistor ohm									//Ohms

extern const float		VREF;			//VDDA 3.3V									//Volts
extern const float		AVG_SLOPE;      // slope (gradient) of temperature line function  [V/°C]
extern const float		V25;         // sensor's voltage at 25°C [V]
extern const float		ADC_TO_VOLT; // conversion coefficient of digital value to voltage [V]

extern uint16_t ADCBuffer[ADCCHANNELS];
extern float VBat, Im, Temp, vSense;
extern int16_t Speed;
extern uint8_t CountMeasure;
extern float AvVBat;
extern float AvTemp;

uint16_t Status1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void GetDateLoop(void);
void SetRTC(void);
void BackupDateToBR(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef TRACE_SWO
uint16_t _write(uint16_t file, char *ptr, uint16_t len)
{
  /* Implement your write code here, this is used by puts and printf for example */
  //int i=0;
  for(uint16_t i=0 ; i<len ; i++) ITM_SendChar((*ptr++));

  return len;
}
#endif


int __io_putchar(int ch) {
    ITM_SendChar(ch);
    return ch;
}
uint8_t StopWaitingDueToPushBtnRelease(void){
	uint8_t PushBtnState;
	bbb_GetBtnStates(&PushBtnState, 0, 0);

	return !PushBtnState;
}


uint8_t StopWaitingDueToPullBtnRelease(void){
	uint8_t PullBtnState;
	bbb_GetBtnStates(0, &PullBtnState, 0);

	return !PullBtnState;
}


uint8_t StopWaitingDueToAnyBtnPressed(void){
	uint8_t PushBtnState, PullBtnState, CfgBtnState;
	bbb_GetBtnStates(&PushBtnState, &PullBtnState, &CfgBtnState);
	return ( PushBtnState || PullBtnState || CfgBtnState);
}

void SaveToEprom()
{
	Status1 = EE_Write(AdrSpeedMode, SpeedMode);
	Status1 = EE_Write(AdrRetractDuration, RetractDuration);
}

void ReadFromEprom()
{
	Status1 = EE_Read(AdrSpeedMode, &SpeedMode);
	Status1 = EE_Read(AdrRetractDuration, &RetractDuration);
}

//Restore variables from EPROM after start if exist, otherwise save default values to EPROM
void RestoreFromEprom()
{
	  /* EEPROM Init */
	  EE_Init();
	  ReadFromEprom();
	  if(SpeedMode == EEPROM_DEFAULT_DATA)
		  { SpeedMode=5; EE_Write(AdrSpeedMode, SpeedMode);}
	  if(RetractDuration == EEPROM_DEFAULT_DATA)
		  {RetractDuration=5; EE_Write(AdrRetractDuration, RetractDuration);}
}

/* The number of user timers is limited by the parent interrupt period.
   Optimally do not exceed 5% of the interruption period for operating
   the number of timers. */
void InitTimers()
{
	Timers[OneSec].Time = 1000; //Uses the named Timer in the timers field this is OneSec Timer
	Timers[OneSec].Tick = 1000;
	Timers[OneSec].Flag.Reload = 1;
	Timers[Milis350].Time = 150; //Uses the named Timer in the timers field this is OneSec Timer
	Timers[Milis350].Tick = 150;
	Timers[Milis350].Flag.Reload = 1;
	Timers[Milis40].Time = 40;//Uses the named Timer in the timers field this is Milis40 Timer
	Timers[Milis40].Tick = 40;
	Timers[Milis40].Flag.Reload = 1;
}

/* Is called ideally in SysTick_Handler interrupt set to 1ms,
   but it can also be called by another periodic routine with a different period.
   Then it is necessary to set the correct value "Time" and "Tick" in "InitTimers()" */
void TimerAction(void)
{
	for(uint8_t i=0;i<(CountTimers);i++) //number of Timers is correct for this code implementation C
	{
	   if(Timers[i].Time==0) continue;  //Time = 0 -> Timer is OFF
	   if(--Timers[i].Tick>0) continue;

	   if(Timers[i].Flag.Reload) Timers[i].Tick = Timers[i].Time; //reload timer count end refresh value
	   else Timers[i].Time=0; // Timer OFF

	   Timers[i].Flag.Over = 1; //Set Over
	   Timers[i].Flag.OverCount++; //INC OverCount
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);
  I2C_DMA_Completed = 1;

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_SlaveTxCpltCallback could be implemented in the user file
   */
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t Error = hi2c->ErrorCode;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_ErrorCallback could be implemented in the user file
   */
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t ConfigWindowTimestamp = 0;

//HAL_GPIO_TogglePin(GPIOA,TEST1_Pin);
//HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_SET);
//HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_RESET);

	//for(volatile uint32_t i=0; i<100000; i++);								//This delay helps probably with oscillator startup issue when it starts with 25% of supposed frequency for some reason (not investigated yet)

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
  m_MX_TIM3_Init();
  InitTimers();

  meas_Init();
  //mo_Init();
  bbb_Init();
  SSD1306_Init();  // initialise

  RestoreFromEprom();

  disp_DrawInitScreen();
  //mo_SetSpeed(PWMPulse(10.0)); //set speed 35.6%
#ifdef TRACE_SWO
	  printf("Set Speed 29.6%%\n");
#endif

  //SetRTC();
  HAL_Delay(200);
  bbb_Melody4(bbb_ToneTd_Low, bbb_ToneTd_Med, bbb_ToneTd_High,bbb_ToneTd_VeryHigh, 2*BEEPDUR);
  bbb_WaitSync(900);
#ifdef TRACE_SWO
	  printf("Melody\n");
#endif
  //SSD1306_MirrorHorizontally(1);
  //SSD1306_MirrorVertically(1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
	{
		HAL_GPIO_TogglePin(GPIOA,TEST_Pin);

		//;GetDateLoop();	//GEt date from RTC and backup to Vbat registers

		if(Timers[Milis350].Flag.Over){
			Timers[Milis350].Flag.Over = 0; //ReSet Over
			HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		}
		if(Timers[Milis40].Flag.Over) {
			Timers[Milis40].Flag.Over = 0; //ReSet Over
			HAL_GPIO_TogglePin(GPIOA,TEST_Pin);
		}

		//HAL_GPIO_TogglePin(GPIOA,TEST_Pin);

		//printf("vbat = %4.2f  ",VBat);
		//printf("VBat = %4.2f  ",VBat * VMCOEF);
		//printf("Temp = %4.2f\n",Temp);

		if(Speed==0)//Motor off, measure IM not need, but VBat+CPUTemp is need start manually
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADCBuffer[0], (uint32_t)ADCCHANNELS);

		if(CountMeasure>=AVGLOOPMAX) // calculate all thermo an print them
		{
			VBat = AvVBat/CountMeasure;
			Temp = AvTemp/CountMeasure;
			CountMeasure = 0;
			RedrawRequested=1; AvVBat=0;AvTemp=0;
		}

		if(RedrawRequested && I2C_DMA_Completed){
		//if(RedrawRequested){
			I2C_DMA_Completed = 0;
			HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_SET);
		    disp_DrawScreen(CfgMode, SpeedModeDesc[SpeedMode], RetractDurationDesc[RetractDuration], VBat, 0, Temp);
		    //disp_DrawScreenDate(CfgMode, SpeedModeDesc[SpeedMode], RetractDurationDesc[RetractDuration], &RtcDate);
		    HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_RESET);
		  RedrawRequested = 0;
		}

		  if(bbb_Update()){																	//Check whether any button has been pressed or released

			  PushBtn = bbb_GetPushBtnState();
			  PullBtn = bbb_GetPullBtnState();
			  CfgBtn = bbb_GetCfgBtnState();


			  if(CfgMode == disp_CfgModeTd_None){											//NORMAL PUSH/PULL OPERATION
				  if(PushBtn->PrevBtnEvt == bbb_EvtTd_Pressed){
					  mo_SetSpeed(PWMPulse(0));														//Stop
					  //bbb_WaitSync(10);														//Wait a moment to stop the movements
					  mo_SetSpeed(PWMPulse(99.9));														//Start push with max speed to null the retraction effect
					  //bbb_WaitSync(RetractDurationList[RetractDuration]);					//Wait for retraction duration
					  bbb_WaitSyncCond(RetractDurationList[RetractDuration], StopWaitingDueToPushBtnRelease);
					  mo_SetSpeed(PWMPulse(SpeedModeList[SpeedMode]));								//Set speed according to settings
					  //bbb_WaitSyncCond(UINT16_MAX, StopWaitingDueToPushBtnRelease);			//Just synchronously wait so the code does not do anything
				  }
				  else if(PushBtn->PrevBtnEvt == bbb_EvtTd_Released || PushBtn->PrevBtnEvt == bbb_EvtTd_LongReleased){
					  mo_SetSpeed(PWMPulse(0));														//Stop
					  //bbb_WaitSync(10);														//Wait a moment to stop the movements
					  mo_SetSpeed(PWMPulse(-99.9));													//Retract
					  //bbb_WaitSync(RetractDurationList[RetractDuration]);					//Still retract
					  bbb_WaitSyncCond(RetractDurationList[RetractDuration], StopWaitingDueToAnyBtnPressed);
					  mo_SetSpeed(PWMPulse(0));
				  }

				  if(PullBtn->PrevBtnEvt == bbb_EvtTd_Pressed){								//When pull button is pressed start motor in reverse max speed
					  mo_SetSpeed(PWMPulse(-99.9));
					  bbb_WaitSyncCond(UINT16_MAX, StopWaitingDueToPullBtnRelease);
				  }
				  else if(PullBtn->PrevBtnEvt == bbb_EvtTd_Released || PullBtn->PrevBtnEvt == bbb_EvtTd_LongReleased){						//When the button is released
					  mo_SetSpeed(PWMPulse(0));
				  }
			  }
			  else{																			//CONFIGURATION MODE
				  if(PushBtn->PrevBtnEvt == bbb_EvtTd_Released){
					  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);
					  if(CfgMode == disp_CfgModeTd_Ret){
						  RetractDuration++; RetractDurationChange = true;
					  }
					  else if(CfgMode == disp_CfgModeTd_Speed){
						  SpeedMode++; SpeedModeChange = true;
					  }
					  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
					  RedrawRequested = 1;
					  mo_SetSpeed(PWMPulse(0));														//Stop
				  }

				  if(PullBtn->PrevBtnEvt == bbb_EvtTd_Released){//When the button is released
					  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);
					  if(CfgMode == disp_CfgModeTd_Ret){
						  RetractDuration--; RetractDurationChange = true;
					  }
					  else if(CfgMode == disp_CfgModeTd_Speed){
						  SpeedMode--; SpeedModeChange = true;
					  }
					  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
					  RedrawRequested = 1;
					  mo_SetSpeed(PWMPulse(0));														//Stop
				  }

				  UT_SATURATE(RetractDuration, 0, (uint16_t)(UT_SIZEOFARRAY(RetractDurationList)-1));
				  UT_SATURATE(SpeedMode, 0, (uint16_t)(UT_SIZEOFARRAY(SpeedModeList)-1));
				  if(SpeedModeChange) {
					  SpeedModeChange = false; EE_Write(AdrSpeedMode, SpeedMode);}
				  if(RetractDurationChange) {
					  RetractDurationChange = false; EE_Write(AdrRetractDuration, RetractDuration);}
			  }

			  if(CfgBtn->PrevBtnEvt == bbb_EvtTd_Released){									//Config button always stops all movements and changes cfg mode
				  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);

				  CfgMode += 1;
				  if(CfgMode > disp_CfgModeTd_Ret) CfgMode = disp_CfgModeTd_None;
				  mo_SetSpeed(PWMPulse(0));													//Stop
				  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
				  RedrawRequested = 1;
				  mo_SetSpeed(PWMPulse(0));														//Stop
			  }
		  }


		  if(ConfigWindowTimestamp && HAL_GetTick() > ConfigWindowTimestamp){
			  ConfigWindowTimestamp = 0;
			  CfgMode = disp_CfgModeTd_None;
			  RedrawRequested = 1;
		  }
	}//end while

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*
void GetDateLoop(void)
{
	HAL_RTC_GetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);

	if(RtcDate.Date != CompareDate){
	  BackupDateToBR();
	  CompareDate = RtcDate.Date;
	}
}

void BackupDateToBR(void)
{
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, ((RtcDate.Date << 8) | (RtcDate.Month)));
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, ((RtcDate.Year << 8) | (RtcDate.WeekDay)));
}

void SetRTC(void)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef DateToUpdate = {0};

	// Initialize RTC and set the Time and Date

	sTime.Hours = 23;
	sTime.Minutes = 10;
	sTime.Seconds = 00;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
	  Error_Handler();
	}

	DateToUpdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	DateToUpdate.Month = RTC_MONTH_JANUARY;
	DateToUpdate.Date = 19;
	DateToUpdate.Year = 21;

	if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
	{
	  Error_Handler();
	}

	BackupDateToBR();
}*/
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
