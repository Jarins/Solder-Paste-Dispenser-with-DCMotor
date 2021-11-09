/*
 * motor.c
 *
 *  Created on: Dec 8, 2019
 *      Author: Standa
 */

#include "motor.h"
#include "utility.h"
#include "tim.h"
#include "main.h"


#define MO_SPEED_MIN	100
#define MO_SPEED_MAX	999


void mo_Init(void){

}


void mo_SetSpeed(int16_t speed){
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);  //PB0 Start pwm second motor 100% duty cycle
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);  //PB1 Start pwm first motor  100% duty cycle

	//HAL_GPIO_ResetOutputPin(PA_GPIO_Port, PA_Pin);
	//HAL_GPIO_ResetOutputPin(PB_GPIO_Port, PB_Pin);
	HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_RESET);   // Stop motor
	HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_RESET);

	if(speed > 0){
		UT_SATURATE(speed, MO_SPEED_MIN, MO_SPEED_MAX);
		//LL_GPIO_ResetOutputPin(PA_GPIO_Port, PA_Pin);
		//LL_GPIO_SetOutputPin(PB_GPIO_Port, PB_Pin);
		HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_SET);

		//HALL_TIM_OC_SetCompareCH1(TIM3, speed);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,speed); //Second motor 75% voltage
		//HAL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
		//HAL_TIM_EnableCounter(TIM3);
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);  //PB0 Start pwm second motor 100% duty cycle
		HAL_TIM_Base_Start(&htim3);
	}
	else if(speed < 0){
		speed *= -1;
		UT_SATURATE(speed, MO_SPEED_MIN, MO_SPEED_MAX);
		//LL_GPIO_SetOutputPin(PA_GPIO_Port, PA_Pin);
		//LL_GPIO_ResetOutputPin(PB_GPIO_Port, PB_Pin);
		HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_RESET);

		//LL_TIM_OC_SetCompareCH2(TIM3, speed);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,speed); //Second motor 75% voltage
		//LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
		//LL_TIM_EnableCounter(TIM3);
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);  //PB0 Start pwm second motor 100% duty cycle
		HAL_TIM_Base_Start(&htim3);
	}
	else{



	}



}
