/*
 * utility.c
 *
 *  Created on: 8 Nov 2017
 *      Author: Standa
 */

#include "utility.h"
#include <sys/types.h>
#include <math.h>


extern float __ieee754_sqrtf __P((float));


//Calculates exponential moving average
//Tau ... time constant
//T ... sampling period
//Alpha = coefficient
//Output = (1-Alpha) * value + Alpha * PrevValue;
//Alpha = exp(-T/Tau);
//Tau = -T/ln(Alpha);
//T = -Tau * ln(Alpha);
float ut_ExpMovAvg(float alpha, float prevVal, float newVal){
	return (1-alpha) * newVal + alpha * prevVal;
}


float ut_CalculateAlpha(float cutOff_Hz, float sampFreq_Hz){
	return exp( -1 * M_TWOPI * cutOff_Hz / sampFreq_Hz );
}


float ut_CalculateAlpha1(float timeConst_s, float sampPeriod_s){
	return exp( -1 * sampPeriod_s / timeConst_s );
}


float ut_CalculateCutOff(float alpha, float sampFreq_Hz){
	return -1 * sampFreq_Hz * log(alpha) /  M_TWOPI;

}


/*
 * Calculates mean and stdev based on stream of values (online)
 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance - Welford's online algorithm
 * Using __ieee754_sqrtf instead sqrtf because it does not link huge ddiv function
 */
void ut_CalculateOnlineMeanStdev(float newValue, ut_MeanStdevTd* result){
	if(result){
		result->Cnt++;													//Increment sample count
		float Delta1 = newValue - result->Mean;
		result->Mean += (Delta1) / result->Cnt;							//Calculate mean
		result->M2 += Delta1 * (newValue - result->Mean);				//Calculate M2
		//storage[1] = sqrtf(storage[2]/storage[3]);					//Calculate StDev
		result->Stdev = __ieee754_sqrtf(result->M2 / result->Cnt);
	}
}


/*
 * Initializes/resets the storage for mean and stdev results and calculation parameters
 */
void ut_InitializeOnlineMeanStdev(ut_MeanStdevTd* result){
	if(result) for(uint8_t i=0; i<sizeof(ut_MeanStdevTd); i++) ((uint8_t*)result)[i] = 0;
}


/*
 * @brief: Converts value in degrees to radians
 */
inline float ut_DegToRad(float deg){
	return deg * 0.0174532925;		//2*PI*1RADIAN = 360deg
}


/*
 * @brief: Converts value in radians to degrees
 */
//Converts value in radians to degrees
inline float ut_RadToDeg(float rad){
	return rad * 57.2957795;		//2*PI*1RADIAN = 360deg
}


/*
 * Converts specified float val into string dest of maximum length maxLen
 * x +-1E6, decimal numbers decCnt from 0 to 3
 * user must make sure that the number will fit into buffer and buffer must be two chars longer
 */
static char Prefixes[] = {'G', 'M', 'K', '.', 'm', 'u', 'n'};
static float Scales[] = {1E9, 1E6, 1E3, 1, 1E-3, 1E-6, 1E-9};

uint16_t ut_ConvFloatToEngStr(float val, char *dest, char *temp, uint8_t maxLen, uint8_t decCnt){
	float ScaledVal;
	uint8_t CharCntr = 0;
    char *s = temp; 									//go to end of buffer
    uint16_t decimals;  										//variable to store the decimals
    int units;  												//variable to store the units (part to left of decimal place)

	if(UT_ISNAN(val)) return 0;									//Check NAN
	if(val >= 1E12 && val <= -1E12) return 0;					//Check range

	uint8_t i;
	for(i=0; i<UT_SIZEOFARRAY(Prefixes); i++){
		ScaledVal = val / Scales[i];
		if( ((int16_t)ScaledVal) >= 1000 || ((int16_t)ScaledVal) <= -1000 ) break;
	}

	if(i==0) return 0;											//Out of upper range
	ScaledVal = val / Scales[i-1];


    if(ScaledVal < 0){ 											//Take care of negative numbers
        decimals = (int32_t)(ScaledVal * -1000) % 1000; 		//Make 1000 for 3 decimals etc.
        units = (int32_t)(-1 * ScaledVal);
    }
    else{ 														//Positive numbers
        decimals = (int32_t)(ScaledVal * 1000) % 1000;
        units = (int32_t)ScaledVal;
    }


	while(decCnt!=0){											//Place all decimal places
		*s++ = (decimals % 10) + '0';
		decimals /= 10; 										//Repeat for as many decimal places as you need
		decCnt--;
		CharCntr++;
	}

	*s++ = Prefixes[i-1];										//Place scaling unit instead of decimal dot
	CharCntr++;

    while (units > 0) {
        *s++ = (units % 10) + '0';
        units /= 10;
        CharCntr++;
    }

    if (ScaledVal < 0){
    	*s++ = '-'; 											//Unary minus sign for negative numbers
    	CharCntr++;
    }

    for(i=0; i<maxLen; i++){
    	if(i==CharCntr){
    		break;
    	}
    	dest[i] = temp[CharCntr-i-1];
    }

    return i;
}


