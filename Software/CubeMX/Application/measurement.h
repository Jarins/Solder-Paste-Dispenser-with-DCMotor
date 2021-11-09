/*
 * measurement.h
 *
 *  Created on: Oct 28, 2019
 *      Author: Standa
 */

#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

#define AVGLOOPMAX 10
#define ADCCHANNELS	3
#define		VMCOEF		 ( ( VMRESH + VMRESL ) / VMRESL ) 		//Coef to convert ADC reading to volts
#define		IMCOEF		  ( VREF / ( 4096.0 * IMSENSR ) ) 			//Coef to convert ADC reading to amps

extern void meas_Init(void);
extern void meas_EndOfADCReadingHandler(void);
extern void meas_AverageCounting(void);

extern float meas_GetVbat(void);
extern float meas_GetIm(void);

#endif /* MEASUREMENT_H_ */
