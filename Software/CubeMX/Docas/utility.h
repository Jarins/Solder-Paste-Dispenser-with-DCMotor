/*
 * utility.h
 *
 *  Created on: 8 Nov 2017
 *      Author: Standa
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdint.h>


#define UT_ISNAN(x)			        ( (x) != (x) )
#define UT_SIZEOFARRAY(x)		    ( sizeof( (x) ) / sizeof( (x[0]) ) )
#define UT_VALINRANGE(val,min,max)  ( ( val >= (min) ) && ( val <= (max) ) )
#define UT_SATURATE(val, min, max)	if( (val) > (max) ){ (val) = (max); }\
									else if( (val) < (min) ){ (val) = (min); }
#define UT_SAFEASSIGN(ptr,val)		if( (ptr) ){ ( *(ptr) = (val) ); };


typedef struct{
	float Mean;
	float Stdev;
	float Cnt;
	float M2;
}ut_MeanStdevTd;


extern float ut_ExpMovAvg(float alpha, float prevVal, float newVal);
extern void ut_InitializeOnlineMeanStdev(ut_MeanStdevTd* result);
extern void ut_CalculateOnlineMeanStdev(float newValue, ut_MeanStdevTd* result);
extern float ut_CalculateAlpha(float cutOff_Hz, float sampFreq_Hz);
extern float ut_CalculateAlpha1(float timeConst_s, float sampPeriod_s);
extern float ut_CalculateCutOff(float alpha, float sampFreq_Hz);

extern float ut_DegToRad(float deg);
extern float ut_RadToDeg(float rad);

extern uint16_t ut_ConvFloatToEngStr(float val, char *dest, char *temp, uint8_t maxLen, uint8_t decCnt);


#endif /* UTILITY_H_ */
