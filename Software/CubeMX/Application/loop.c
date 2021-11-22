/*
 * Code.c
 *
 *  Created on: 21. 11. 2021
 *      Author: Jarek
 */

#pragma GCC diagnostic ignored "-Wunused-variable"

#include "adc.h"
#include "tim.h"

#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include "stm32f1xx_hal_def.h"
#include "fonts.h"
#include "motor.h"
#include "measurement.h"
#include <ButtonBeep.h>
#include "utility.h"
#include "stm32f1xx_it.h"
#include "ssd1306.h"
#include "display.h"
#include "eeprom.h"



#define BEEPDUR	50
#define CONFIGWINDOWDURATION_MS	2000
#define BBB_LONGPRESS_MS			400

typedef void (* F_RUN)(); // Variable type for the indirect call function

/* Private variables ---------------------------------------------------------*/
uint32_t ConfigWindowTimestamp = 0;

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
	Button10,
	FunctionDelay,
	CountTimers // is not in the set of active elements, only for element count in ENUM
};

volatile TIMER Timers[CountTimers]; //number of Timers is correct for this code implementation C
BtnState *pBtnPUSH;
BtnState *pBtnPULL;
BtnState *pBtnCFG;

static int16_t SpeedMode = 5;
static const uint16_t AdrSpeedMode = 0x1111; //virtual address to EPROM
bool SpeedModeChange = false;

static int16_t RetractDuration = 5;
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

F_RUN RunFunction; // Variable for the indirect call function

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void GetDateLoop(void);
void SetRTC(void);
void BackupDateToBR(void);
void MotorPUSH1(void);
void MotorPUSH2(void);
void MotorUnPUSH1(void);
void MotorUnPUSH2(void);
void WaitUntilPUSHKeyPressed(void);
void WaitUntilPUSHKeyReleased(void);
void WaitUntilPULLKeyPressed(void);
void WaitUntilPULLKeyReleased(void);

void SelOperation(void);
void Motor(void);
void Config(void);

void Nil(){};

// CODE -------------------------------------------------------------------------

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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	for(uint8_t i=0;i<UT_SIZEOFARRAY(ButList); i++) {
		if (GPIO_Pin == ButList[i]->Pin && !ButList[i]->IntStart)  {
			ButList[i]->IntStart = true;
			Timers[Button10].Control.Run = 1; //Start Timer to 10ms
			return;
		}
	}
}

uint8_t StopWaitingDueToPushBtnRelease(void){
	//uint8_t PushBtnState;
	//bbb_GetBtnStates(&PushBtnState, 0, 0);

	return 0;//!PushBtnState;
}

uint8_t StopWaitingDueToPullBtnRelease(void){
	//uint8_t PullBtnState;
	//bbb_GetBtnStates(0, &PullBtnState, 0);

	return 0;//!PullBtnState;
}

uint8_t StopWaitingDueToAnyBtnPressed(void){
	//uint8_t PushBtnState, PullBtnState, CfgBtnState;
	//bbb_GetBtnStates(&PushBtnState, &PullBtnState, &CfgBtnState);
	return 1;//( PushBtnState || PullBtnState || CfgBtnState);
}

void SaveToEprom()
{
	EE_Write(AdrSpeedMode, SpeedMode);
	EE_Write(AdrRetractDuration, RetractDuration);
}

void ReadFromEprom()
{
	EE_Read(AdrSpeedMode, &SpeedMode);
	EE_Read(AdrRetractDuration, &RetractDuration);
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
	Timers[OneSec].Value = 1000; //Uses the named Timer in the timers field this is OneSec Timer
	Timers[OneSec].Tick = 0;
	Timers[OneSec].Control.Reload = 1;
	Timers[OneSec].Control.Run = 1;	//Init timer to Start mode

	Timers[Button10].Value = 10; // 10ms value
	Timers[Button10].Tick = 0;
	Timers[Button10].Control.Reload = 0; // OneShot Timer No Reload
	Timers[Button10].Control.Run = 0;	//Init Timer to Stop

	Timers[FunctionDelay].Value = 0; //Init Timer to Stop mode
	Timers[FunctionDelay].Tick = 0;
	Timers[FunctionDelay].Control.Over = 1; // For first shot after Setup
	Timers[FunctionDelay].Control.Reload = 0; // OneShot Timer No Reload
	Timers[FunctionDelay].Control.Run = 0;	//Stop timer

}

/* Is called ideally in SysTick_Handler interrupt set to 1ms,
   but it can also be called by another periodic routine with a different period.
   Then it is necessary to set the correct value "Value" in "InitTimers()" */
void TimerAction(void)
{
	for(uint8_t i=0;i<(CountTimers);i++) //number of Timers is correct for this code implementation C
	{
	   if(Timers[i].Control.Run==0) continue;  //Run = 0 -> Timer is OFF
	   if(Timers[i].Value==0) continue;  //Value = 0 -> Timer is OFF

	   if(Timers[i].Tick==0) Timers[i].Tick = Timers[i].Value; //Fill timer count value after start
	   if(--Timers[i].Tick>0) continue;

	   if(Timers[i].Control.Reload) Timers[i].Tick = Timers[i].Value; //reload timer with refresh value
	   else Timers[i].Control.Run=0; // No reload Timer OFF

	   Timers[i].Control.Over = 1; //Set Over
	   Timers[i].Control.OverCount++; //INC OverCount
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

void Buttons(void)
{// Debouncing Time for key over
	if(Timers[Button10].Control.Over==0) return;

	Timers[Button10].Control.Over = 0; //ReSet Over
	for(uint8_t i=0;i<UT_SIZEOFARRAY(ButList); i++) {
		if(ButList[i]->IntStart){
			ButList[i]->IntStart = false; // Clear state for next action with key
			if ((ButList[i]->Port->IDR & ButList[i]->Pin)==0) ButList[i]->LastState = But_Pressed;//Rising so pressed
			else ButList[i]->LastState = But_Released;//Falling so released
		}
	}
}
void Display(void)
{
	if(RedrawRequested && I2C_DMA_Completed){
		I2C_DMA_Completed = 0;
		HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_SET);// for oscilloscope measure rising edge ___|¯¯¯
		disp_DrawScreen(CfgMode, disp_SpeedDesc(SpeedMode), disp_RetractDurationDesc(RetractDuration), VBat, 0, Temp);
	   HAL_GPIO_WritePin(GPIOA,TEST1_Pin,GPIO_PIN_RESET);// for oscilloscope measure falling edge ¯¯¯|___
	   RedrawRequested = 0;
	}
}
void Meassuring(void)
{
	//Motor off, TIM3 is trigger for ADC and is stopped, measure IM not need, but VBat+CPUTemp is need start manually
	if(Speed==0 && I2C_DMA_Completed) HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADCBuffer[0], (uint32_t)ADCCHANNELS);

	if(CountMeasure<AVGLOOPMAX) return;// calculate all thermo an print them

	VBat = AvVBat/CountMeasure;
	Temp = AvTemp/CountMeasure;
	CountMeasure = AvVBat = AvTemp = 0; RedrawRequested=1;
}
void Motor(void)
{
	if(Timers[FunctionDelay].Control.Over){
		Timers[FunctionDelay].Control.Over=0;
		RunFunction();
	}
	else if(Timers[FunctionDelay].Control.Run==0) RunFunction(); //No wait for Timer Run immediately
}
void MotorPUSH1(void)
{
	mo_SetSpeed(PWMPulse(SpeedMode*5));
	RunFunction = &WaitUntilPUSHKeyPressed; //Next function is waiting to released PUSH button after 130ms
	Timers[FunctionDelay].Value = 130; //Wait for retraction duration half time
	Timers[FunctionDelay].Control.Run=1; //Start Timer;
}
void MotorPUSH2(void)
{
	  mo_SetSpeed(PWMPulse(0));														//Stop
	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
	  RunFunction = &SelOperation; //Return to Select Operation in next Call
}
void MotorUnPUSH1(void)
{
	mo_SetSpeed(PWMPulse(-99.9));													//Retract
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET); // LED ON
	RunFunction = &MotorUnPUSH2; //Next function is MotorUnPUSH2 after RetractDuration*100ms
	Timers[FunctionDelay].Value = RetractDuration*100; //Wait for retraction duration half time
	Timers[FunctionDelay].Control.Run=1; //Start Timer;
}







void MotorUnPUSH2(void)
{
	mo_SetSpeed(PWMPulse(0));
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
	pBtnPUSH->LastState = But_None;
	RunFunction = &SelOperation; //Return to Select Operation in next Call
}
void WaitUntilPUSHKeyPressed(void) //wait until the PUSH key is pressed, but no blocking LOOP
{
	//Next function is MotorPUSH2 after released PUSH button
	if(pBtnPUSH->LastState != But_Pressed) RunFunction = &MotorPUSH2;
}
void WaitUntilPULLKeyPressed(void) //wait until the PULL key is pressed, but no blocking LOOP
{
	//Next function is MotorPUSH2 after released PUSH button
	if(pBtnPULL->LastState != But_Pressed) RunFunction = &SelOperation;
}

void SelOperation(void)
{
	if(CfgMode == disp_CfgModeTd_None)//MOTOR PUSH/PULL OPERATION
	{
		if(pBtnPUSH->LastState == But_Pressed)
		{
		  mo_SetSpeed(PWMPulse(99.9));		//Start push with max speed to null the retraction effect
		  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET); // LED ON
		  RunFunction = &MotorPUSH1; //Next function is MotorPUSH1 after RetractDuration*100ms
		  Timers[FunctionDelay].Value = RetractDuration*100; //Wait for retraction duration half time
		  Timers[FunctionDelay].Control.Run=1; //Start Timer;
		  return;
		}
		else if(pBtnPUSH->LastState == But_Released)
		{
		  mo_SetSpeed(PWMPulse(0));														//Stop
		  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
		  RunFunction = &MotorUnPUSH1; //Next function is MotorUnPUSH1 after 10ms
		  Timers[FunctionDelay].Value = 10; //Wait for stop
		  Timers[FunctionDelay].Control.Run=1; //Start Timer;
		  return;
		}

		if(pBtnPULL->LastState == But_Pressed)
		{								//When pull button is pressed start motor in reverse max speed
		  mo_SetSpeed(PWMPulse(-99.9));
		  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET); // LED ON
		  RunFunction = &WaitUntilPULLKeyPressed;//Next function is waiting to released PULL button
		}
		else if(pBtnPULL->LastState == But_Released)
		{						//When the button is released
		  mo_SetSpeed(PWMPulse(0));
		  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
		  pBtnPULL->LastState = But_None;
		}
	}
	else
	{//CONFIGURATION MODE
		if(pBtnPUSH->LastState == But_Released)
		{
		  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);
		  if(CfgMode == disp_CfgModeTd_Ret)
		  {
			  if(++RetractDuration>30) RetractDuration=30;
			  RetractDurationChange = true;
		  }
		  else if(CfgMode == disp_CfgModeTd_Speed)
		  {
			  if(++SpeedMode>20) SpeedMode=20;
			  SpeedModeChange = true;
		  }
		  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
		  RedrawRequested = 1;
		  //mo_SetSpeed(PWMPulse(0));														//Stop
		  //HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
		  pBtnPUSH->LastState = But_None;
		}

		if(pBtnPULL->LastState == But_Released)
		{//When the button is released
		  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);
		  if(CfgMode == disp_CfgModeTd_Ret)
		  {
			  if(--RetractDuration < 0) RetractDuration=0;
			  RetractDurationChange = true;
		  }
		  else if(CfgMode == disp_CfgModeTd_Speed)
		  {
			  if(--SpeedMode < 1) SpeedMode=1;
			  SpeedModeChange = true;
		  }
		  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
		  RedrawRequested = 1;
		  //mo_SetSpeed(PWMPulse(0));														//Stop
		  //HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
		  pBtnPULL->LastState = But_None;
		}

		if(SpeedModeChange) {SpeedModeChange = false; EE_Write(AdrSpeedMode, SpeedMode);}
		if(RetractDurationChange) {RetractDurationChange = false; EE_Write(AdrRetractDuration, RetractDuration);}
	}
}

void Config(void)
{
	if(pBtnCFG->LastState == But_Pressed){									//Config button always stops all movements and changes cfg mode
	  Timers[FunctionDelay].Control.Run=0; //Stop Timer;
	  mo_SetSpeed(PWMPulse(0));												//Stop
	  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET); // LED OFF
	  RunFunction = &SelOperation; //Return to Select Operation in next Call
	  bbb_Beep(bbb_ToneTd_Low, 2*BEEPDUR);

	  CfgMode += 1;
	  if(CfgMode > disp_CfgModeTd_Ret) CfgMode = disp_CfgModeTd_None;
	  ConfigWindowTimestamp = HAL_GetTick() + CONFIGWINDOWDURATION_MS;
	  RedrawRequested = 1;
	  pBtnCFG->LastState = But_None;
	}

	if(ConfigWindowTimestamp && HAL_GetTick() > ConfigWindowTimestamp){
		  ConfigWindowTimestamp = 0;
		  CfgMode = disp_CfgModeTd_None;
		  RedrawRequested = 1;
	}
}

void Setup(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	m_MX_TIM3_Init();
	InitTimers();
	meas_Init();

	// ??? it must be remodeled
	pBtnPUSH = ButList[PUSH]; pBtnPULL = ButList[PULL]; pBtnCFG = ButList[CFG];

	SSD1306_Init();  // initialise

	RestoreFromEprom();

	disp_DrawInitScreen();
	//SetRTC();
	bbb_Melody4(bbb_ToneTd_Low, bbb_ToneTd_Med, bbb_ToneTd_High,bbb_ToneTd_VeryHigh, 2*BEEPDUR);
	HAL_Delay(1500);
	RunFunction = &SelOperation;
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);//LED OFF
}
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
