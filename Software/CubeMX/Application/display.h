/*
 * display.h
 *
 *  Created on: Nov 27, 2019
 *      Author: Standa
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
#include "main.h"

typedef enum{
	disp_CfgModeTd_None = 0,
	disp_CfgModeTd_Speed = 1,
	disp_CfgModeTd_Ret = 2,
	disp_CfgModeTd_Pulse = 3,
}disp_CfgModeTd;

extern char* disp_PulseDesc(int16_t pulse);
extern char* disp_RetractDurationDesc(int16_t Duration);
extern char* disp_SpeedDesc(int16_t Speed);
extern void disp_DrawSavedPopup(void);
extern void disp_DrawInitScreen(void);
extern void disp_DrawScreen(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, char* pushPulse, float vBat, float iMeas, float tCore);
//extern void disp_DrawScreenDate(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, RTC_DateTypeDef *rtcDate);


#endif /* DISPLAY_H_ */
