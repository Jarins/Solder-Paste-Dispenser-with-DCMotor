/*
 * measurement.c
 *
 *  Created on: Oct 28, 2019
 *      Author: Standa
 *
 *	This module provides fixed count (ADC_READSPERMEASPERIOD) of fast analog measurements of VM10R, VM100R and IM channels during single period (1/freq_Hz).
 *	Secondary function is to provide monitoring of VM10R, VM100R, IM, VBAT, VREF (for VDDA calculation) and TEMP when fast measurement is not active.
 *	TIM1 is the counter reserved for ADC conversion triggering and DMA1 CH1 used to transfer the converted data into memory.
 */
#include <stdio.h>
#include "measurement.h"
#include "error.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"

/*
Internal temperature calculation formula:

【Temperature = {(V25 - VSENSE) / Avg_Slope} + 25】
Temperature (in °C) = {(V25 - VSENSE) / Avg_Slope} + 25.
V25： 1.34V minimum, 1.52V maximum, 1.43V typical
Avg_Slope： Minimum 4.0, maximum 4.6, typical value 4.3mv/℃
VSENSE： Voltage collected by ADC
ADC value to voltage value calculation formula: [Voltage = value collected by ADC * 3.3 / 4096]
Looking at the picture above, the value I collected is 1703, first convert it to a voltage value: 1703*3.3/4096≈1.37
Converted to temperature: (1.43-1.37) / 4.3 + 25 ≈ 25.01℃


The STM32 series chips are mostly 12-bit and only a small part is 16-bit, such as the F373 chip.
The 12-bit resolution means that the accuracy of our acquisition voltage can reach: Vref / 4096.

Acquisition voltage = Vref * ADC_DR / 4096;
VREF: reference voltage
ADC_DR: Read the value into the ADC data register
*/

const float		VMRESH		= 51000.0;	//Rezistor R1 from Voltage divider									//Ohms
const float		VMRESL		= 5600.0;	//Rezistor R2 from Voltage divider								//Ohms

const float		IMSENSR		= 10.0; //Measure rezistor ohm									//Ohms

const float		VREF		= 3.3;			//VDDA 3.3V									//Volts
const float		AVG_SLOPE   = 4.3E-03;      // slope (gradient) of temperature line function  [V/°C]
const float		V25         = 1.43;         // sensor's voltage at 25°C [V]
const float		ADC_TO_VOLT = 3.3 / 4096.0; // conversion coefficient of digital value to voltage [V]

//const float		VMCOEF		 =( ( VMRESH + VMRESL ) / VMRESL ); 		//Coef to convert ADC reading to volts
//const float		IMCOEF		 =( VREF / ( 4096.0 * IMSENSR ) ); 			//Coef to convert ADC reading to amps


volatile uint16_t ADCBuffer[ADCCHANNELS];
extern uint8_t RedrawRequested;
volatile float VBat, Im, Temp, vSense;
volatile float AvVBat = 0;
volatile float AvTemp = 0;
volatile uint8_t IsMeasure = 0;
volatile uint8_t CountMeasure = 0;



/*
 * Initialize generator module, used typically during boot only
 */
void meas_Init(void){

	VBat = 0;
	Im = 0;
	Temp = 0;
	vSense = 0;
	//HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);  //PA_Pin Start pwm motor speed% duty cycle
	//HAL_TIM_Base_Start(&htim1);
	// Kalibrace ADC při zapnutí pro lepší přesnost
	 while(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK);           // calibrate AD convertor
	// Start ADC Conversion
	// Pass (The ADC Instance, Result Buffer Address, Buffer Length)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADCBuffer[0], (uint32_t)ADCCHANNELS);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	//HAL_GPIO_TogglePin(GPIOA,TEST_Pin);
	if(CountMeasure>AVGLOOPMAX) return;
	AvVBat += (ADC_TO_VOLT * ADCBuffer[0]);// * VMCOEF;
	//Im = ADCBuffer[1] * IMCOEF;
	vSense = (ADC_TO_VOLT * ADCBuffer[2]);// volts temperature;
	AvTemp += (V25 - vSense) / AVG_SLOPE + 25.0f;                  // convert sensor's output voltage to temperature [°C]
	ADCBuffer[0] = ADCBuffer[1] = ADCBuffer[2] = 0;
	CountMeasure++;
}
/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  /* Turn LED5 on: Transfer Error */
	printf("DMA Transfer Error\n");
}

float meas_GetVbat(void){
	return VBat;
}


float meas_GetIm(void){
	return Im;
}
