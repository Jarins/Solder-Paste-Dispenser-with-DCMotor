/*
 * bbb.c
 *
 *  Created on: Nov 25, 2019
 *      Author: Standa
 */

#include "bbb.h"
#include "stm32f1xx_it.h"
#include <stdint.h>
#include "measurement.h"
#include "tim.h"
#include "main.h"
#include "utility.h"



#define BBB_UPDATEINTERVAL_MS		25
#define BBB_LONGPRESS_MS			400
#define BBB_DOUBLEPRESS_MS			400


static bbb_BtnTd PushBtn = {BPUSH_GPIO_Port, BPUSH_Pin, 1, 0, 0};
static bbb_BtnTd PullBtn = {BPULL_GPIO_Port, BPULL_Pin, 1, 0, 0};
static bbb_BtnTd CfgBtn = {BCFG_GPIO_Port, BCFG_Pin, 1, 0, 0};

static bbb_BtnTd* BtnList[] = {&PushBtn, &PullBtn, &CfgBtn};
static uint32_t LastBtnEvtUpdateTimestamp = 0;


static uint8_t BuzzerActive = 0;
static float VBat = 0;
static float Temp = 0;

/*
 * Initialize button, buzzer, battery module
 */
void bbb_Init(void){
	//HAL_TIMEx_PWMN_Start (& htim1, TIM_CHANNEL_3);
}

/*
 * Updates all bbb readings. Handles debouncing.
 * Returns 1 when button update is evaluated, returns 0 when it has been evaluated too recently (earlier than BBB_UPDATEINTERVAL_MS)
 */
uint8_t bbb_Update(void){
	bbb_BtnTd* pBtn;


	if( HAL_GetTick() < LastBtnEvtUpdateTimestamp + BBB_UPDATEINTERVAL_MS ){						//If the buttons have been updated recently and no action is done
		for(uint8_t i=0; i< UT_SIZEOFARRAY(BtnList); i++){										//Set none event to each of buttons
			pBtn = BtnList[i];
			if(pBtn == 0) continue;
			pBtn->PrevBtnEvt = bbb_EvtTd_None;
		}
		return 0;																				//And just return with zero indicating no update happened
	}


	LastBtnEvtUpdateTimestamp = HAL_GetTick();														//Store last update timestamp

	for(uint8_t i=0; i< UT_SIZEOFARRAY(BtnList); i++){											//Update status of all the buttons in the list
		pBtn = BtnList[i];																		//Just a shortcut for nicer and shorter code - pointer to currently analyzed button
		if(pBtn == 0) continue;																	//Make sure the handle exists

		if(BtnList[i]->Port->IDR & BtnList[i]->Pin) {							//Input with pullup, switch to gnd (0 when pressed, 1 when released). !!!BUT!!! below in the code 0 means not pressed, 1 means pressed - called "normalized logic"
			if(pBtn->PrevBtnState != 0){														//If button state is different from last reading (released)
				pBtn->PrevBtnState = 0;															//Save current state - already normalized logic
				uint32_t ReleasedToReleased = HAL_GetTick() - pBtn->ReleasedTimestamp;				//Delay between this and previous release (falling edge to falling edge)
				pBtn->ReleasedTimestamp = HAL_GetTick();
				pBtn->StateChangeInterval = HAL_GetTick() - pBtn->PrevBtnStateTimestamp;			//Calculate press duration
				pBtn->PrevBtnStateTimestamp = HAL_GetTick();										//Update state change timestamp

				if(pBtn->StateChangeInterval > BBB_DOUBLEPRESS_MS){
					pBtn->SingleReleasedEnsuredTimestamp = 0;
					pBtn->PrevBtnEvt = bbb_EvtTd_LongReleased;
				}
				else if(ReleasedToReleased < BBB_DOUBLEPRESS_MS){
					pBtn->SingleReleasedEnsuredTimestamp = 0;
					pBtn->PrevBtnEvt =  bbb_EvtTd_DoubleReleased;
				}
				else{
					pBtn->SingleReleasedEnsuredTimestamp = HAL_GetTick()+BBB_DOUBLEPRESS_MS;
					pBtn->PrevBtnEvt =  bbb_EvtTd_Released;
				}
			}
			else{
				if(pBtn->SingleReleasedEnsuredTimestamp && HAL_GetTick() > pBtn->SingleReleasedEnsuredTimestamp){
					pBtn->SingleReleasedEnsuredTimestamp = 0;
					pBtn->PrevBtnEvt =  bbb_EvtTd_SingleReleased;
				}
				else{
					pBtn->PrevBtnEvt = bbb_EvtTd_Low;
				}
			}
		}
		else{																					//Actual pin status is zero which means normalized state is 1 :)
			if(pBtn->PrevBtnState != 1){														//If button state is different from last reading (now is pressed, before was not pressed)
				pBtn->PrevBtnState = 1;															//Save current state
				pBtn->StateChangeInterval = HAL_GetTick() - pBtn->PrevBtnStateTimestamp;
				pBtn->PrevBtnStateTimestamp = HAL_GetTick();
				pBtn->PrevBtnEvt = bbb_EvtTd_Pressed;
			}
			else{
				pBtn->PrevBtnEvt = bbb_EvtTd_High;
			}
		}
	}
	return 1;
}


void bbb_GetBtnStates(uint8_t* pushBtn, uint8_t* pullBtn, uint8_t *cfgBtn){
	UT_SAFEASSIGN(pushBtn, !(PushBtn.Port->IDR & PushBtn.Pin));
	UT_SAFEASSIGN(pullBtn, !(PullBtn.Port->IDR & PushBtn.Pin));
	UT_SAFEASSIGN(cfgBtn, !(CfgBtn.Port->IDR & PushBtn.Pin));
}


bbb_BtnTd* bbb_GetPushBtnState(void){
	return &PushBtn;
}


bbb_BtnTd* bbb_GetPullBtnState(void){
	return &PullBtn;
}


bbb_BtnTd* bbb_GetCfgBtnState(void){
	return &CfgBtn;
}


void bbb_WaitSync(uint16_t waitMs){
	uint32_t End = HAL_GetTick() + waitMs;

	while( HAL_GetTick() < End );
}


void bbb_WaitSyncCond(uint16_t waitMs, uint8_t (*stopRequestFn)(void)){
	uint32_t End = HAL_GetTick() + waitMs;

	while(HAL_GetTick() < End){
		if(stopRequestFn && stopRequestFn()) return;
	}
}

/*
 * Beeps the buzzer with specified tone (0 = off, 1=low, 2=medium, 3=high) for specified time in ms
 * tone 0 with time is just delay
 */
void bbb_Beep(uint8_t tone, uint8_t time){
	volatile uint32_t Stop = HAL_GetTick() + time;

	BuzzerActive = 1;
	if(tone > 4) tone = 4;

	if(tone > 0){
		//Start timer
		//LL_TIM_SetAutoReload(TIM14, 1200 - tone * 200);
		__HAL_TIM_SET_AUTORELOAD(&htim1, 1200 - tone * 200);
		//LL_TIM_OC_SetCompareCH4(TIM14, (1200 - tone * 200)>>1);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,(1200 - tone * 200)>>1);
		//LL_TIM_CC_EnableChannel(TIM14, LL_TIM_CHANNEL_CH4);
		HAL_TIMEx_PWMN_Start (&htim1, TIM_CHANNEL_3);
		//HAL_TIM_Base_Start(&htim1);
	}

	while(1) if(HAL_GetTick() > Stop) break;

	if(tone > 0){
		//Stop timer
		//LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH4);
		HAL_TIMEx_PWMN_Stop (&htim1, TIM_CHANNEL_3);
	}


	BuzzerActive = 0;
}


void bbb_Melody2(uint8_t tone1, uint8_t tone2, uint16_t time){
	  bbb_Beep(tone1, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone2, time);
	  bbb_Beep(0, time);
}


void bbb_Melody3(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint16_t time){
	  bbb_Beep(tone1, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone2, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone3, time);
	  bbb_Beep(0, time);
}

void bbb_Melody4(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint8_t tone4, uint16_t time){
	  bbb_Beep(tone1, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone2, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone3, time);
	  bbb_Beep(0, time);
	  bbb_Beep(tone4, time);
	  bbb_Beep(0, time);
}


float bbb_GetVbat(void){
	return VBat;
}

float bbb_GetTemp(void){
	return Temp;
}


