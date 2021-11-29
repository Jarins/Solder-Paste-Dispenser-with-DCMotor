/*
 * ButtonBeep.c
 *
 *  Created on: Nov 25, 2019
 *      Author: Standa
 *      reprogrammed Nov 22, 2021
 *      Author Jarin
 */

#include <ButtonBeep.h>
#include "stm32f1xx_it.h"
#include <stdint.h>
#include "measurement.h"
#include "tim.h"
#include "main.h"
#include "utility.h"

static BtnState BtnPUSH = {false,But_None,BPUSH_GPIO_Port, BPUSH_Pin};
static BtnState BtnPULL = {false,But_None,BPULL_GPIO_Port, BPULL_Pin};
static BtnState BtnCFG = {false,But_None,BCFG_GPIO_Port, BCFG_Pin};
volatile BtnState* ButList[] = {&BtnPUSH, &BtnPULL, &BtnCFG};

static uint8_t BuzzerActive = 0;

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
		__HAL_TIM_SET_AUTORELOAD(&htim1, 1200 - tone * 200);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,(1200 - tone * 200)>>1);
		HAL_TIMEx_PWMN_Start (&htim1, TIM_CHANNEL_3);
	}

	while(1) if(HAL_GetTick() > Stop) break;

	if(tone > 0){
		//Stop timer
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
