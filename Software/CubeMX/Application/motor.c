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


#define MO_SPEED_MIN	80//100
#define MO_SPEED_MAX	800//999

volatile int16_t Speed;


void mo_Init(void){

}


void mo_SetSpeed(int16_t speed){

	Speed = speed;

	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);  //PA_Pin Stop pwm
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);  //PB_Pin Stop pwm

	HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_RESET);   // Stop motor
	HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_RESET);

	if(speed > 0){
		UT_SATURATE(speed, MO_SPEED_MIN, MO_SPEED_MAX);
		HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_SET);

		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,speed); //Set motor speed% voltage
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);  //PA_Pin Start pwm motor speed% duty cycle
		HAL_TIM_Base_Start(&htim3);
	}
	else if(speed < 0){
		speed *= -1;
		UT_SATURATE(speed, MO_SPEED_MIN, MO_SPEED_MAX);
		HAL_GPIO_WritePin(PA_GPIO_Port,PA_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(PB_GPIO_Port,PB_Pin,GPIO_PIN_RESET);

		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,speed); //Set motor speed% voltage
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);  //PB_Pin Start pwm motor speed% duty cycle
		HAL_TIM_Base_Start(&htim3);
	}
	else{



	}



}
