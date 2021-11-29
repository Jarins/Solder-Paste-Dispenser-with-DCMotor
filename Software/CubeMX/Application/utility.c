/*
 * utility.c
 *
 *  Created on: 8 Nov 2017
 *      Author: Standa
 */

#include "utility.h"
#include <stdlib.h>
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

//sprintf () is too slow, this function is many times (hundred times) faster
uint8_t floatToString(char * outstr, float value, uint8_t places, uint8_t minwidth, uint8_t noleadzero)
{
    // this is used to write a float value to string, outstr.  outstr is also the return value.
	uint8_t digit;
    float tens = 0.1;
    uint8_t tenscount = 0;
    uint8_t i;
    float tempfloat = value;
    uint8_t c = 0;
    uint8_t charcount = 1;
    uint8_t extra = 0;
    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
    // if this rounding step isn't here, the value  54.321 prints as 54.3209

    // calculate rounding term d:   0.5/pow(10,places)
    float d = 0.5;
    if (value < 0) d *= -1.0;
    // divide by ten for each decimal place
    for (i = 0; i < places; i++) d/= 10.0;
    // this small addition, combined with truncation will round our values properly
    tempfloat +=  d;
    // first get value tens to be the large power of ten less than value
    if (value < 0) tempfloat *= -1.0;
    while ((tens * 10.0) <= tempfloat) {
        tens *= 10.0;
        tenscount += 1;
    }

    if (tenscount > 0) charcount += tenscount;
    else charcount += 1;

    if (value < 0) charcount += 1;
    charcount += 1 + places;
    minwidth += 1; // both count the null final character
    if (minwidth > charcount){
        extra = minwidth - charcount;
        charcount = minwidth;
    }
    // write out the negative if needed
    if (value < 0) outstr[c++] = '-';
    if (tenscount == 0 && noleadzero==0) outstr[c++] = '0';
    for (i=0; i< tenscount; i++) {
        digit = (int) (tempfloat/tens);
        itoa(digit, &outstr[c++], 10);
        tempfloat = tempfloat - ((float)digit * tens);
        tens /= 10.0;
    }

    // if no places after decimal, stop now and return
    // otherwise, write the point and continue on
    if (places > 0) outstr[c++] = '.';

    // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
    for (i = 0; i < places; i++) {
        tempfloat *= 10.0;
        digit = (int) tempfloat;
        itoa(digit, &outstr[c++], 10);
        // once written, subtract off that digit
        tempfloat = tempfloat - (float) digit;
    }
    if (extra > 0) {
        for (int i = 0; i< extra; i++) {
            outstr[c++] = ' ';
        }
    }
    outstr[c++] = '\0';
    return charcount;
}
