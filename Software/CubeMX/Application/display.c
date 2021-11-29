/*
 * display.c
 *
 *  Created on: Nov 27, 2019
 *      Author: Standa
 */

#include "display.h"
#include "SSD1306.h"
#include "utility.h"
#include <string.h>
#include "gpio.h"



//INIT SCREEN INFO --- 18 char per line max
#define DEVSTRING  "DCMotor dispenser"
#define NAMESTRING " By Jarin (c)2021"
#define REVSTR     "  v2.21.11.3010"

#define TMPSIZE	21	//18 is the length of line
char Tmp[TMPSIZE];
char Line[TMPSIZE];
char floatBuf[10]; //temp buffer for float conversion
char PulseText[6];
char DurationText[6];
char SpeedText[6];

char* disp_PulseDesc(int16_t pulse)
{
	if(pulse>0)
	{
		floatToString(floatBuf, (float)((pulse*100)/1000.0), 1, 2, 1);
		strcpy(PulseText,floatBuf);
		strcat(PulseText,"s");
	}
	else strcpy(PulseText,"Dis ");

	return PulseText;
}
char* disp_RetractDurationDesc(int16_t Duration)
{
	if(Duration>0)
	{
		floatToString(floatBuf, (float)((Duration*100)/1000.0), 1, 3, 0);
		strcpy(DurationText,floatBuf);
		strcat(DurationText,"s");
	}
	else strcpy(DurationText,"Dis ");

	return DurationText;
}
char* disp_SpeedDesc(int16_t Speed)
{
	itoa(Speed*5,SpeedText,10);
	strcat(SpeedText,"%");
	return SpeedText;
}

/*
 * Draws frame around specified area (optionally filled with black color)
 */
void disp_DrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t isFilled){

	for(uint8_t i=y1; i<=y2; i++){
		if(isFilled) for(uint8_t j=x1; j<=x2; j++) SSD1306_DrawPixel(j, i, SSD1306_COLOR_BLACK);
		SSD1306_DrawPixel(x1, i, SSD1306_COLOR_WHITE);
		SSD1306_DrawPixel(x2, i, SSD1306_COLOR_WHITE);
	}

	for(uint8_t i=x1; i<=x2; i++){
		SSD1306_DrawPixel(i, y1, SSD1306_COLOR_WHITE);
		SSD1306_DrawPixel(i, y2, SSD1306_COLOR_WHITE);
	}
}

/*
 * Draws "saved" pop-up window in the center of the screen
 */
void disp_DrawSavedPopup(void){

	  //SECOND LINE
	strcpy(Line + 0, "      Saved!      ");
	disp_DrawFrame(26, 8, 100, 22, 1);
	SSD1306_GotoXY(28, 10);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
}

/*
 * Draws loading screen with battery voltage and core temperature
 */
#define Start_Y 0//16
void disp_DrawInitScreen(void){
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	disp_DrawFrame(0, 0, 125, Start_Y+31, 0);
	//SSD1306_DrawRectangle(0, 0, 127, Start_Y+31,SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(1, Start_Y+1);
	SSD1306_Puts(DEVSTRING, &Font_7x10, SSD1306_COLOR_WHITE);

	SSD1306_GotoXY(1, Start_Y + 11);
	SSD1306_Puts(NAMESTRING, &Font_7x10, SSD1306_COLOR_WHITE);

	SSD1306_GotoXY(1, Start_Y + 21);
	SSD1306_Puts(REVSTR, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
}

/*

 * Draws normal screen, cfg mode specifies which configuration is highlighted, pushspeed, pullduration is string representation of selected parameters
 */
void disp_DrawScreen(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, char* pushPulse, float vBat, float iMeas, float tCore){

	SSD1306_Fill(SSD1306_COLOR_BLACK);
	//disp_DrawFrame(0, 0, 127, Start_Y+31, 0);

	//FIRST LINE
	strcpy(Line," Speed ");
	strcat(Line,pushSpeed);
	SSD1306_GotoXY(1, Start_Y+0);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//SECOND LINE
	strcpy(Line," Retr ");
	strcat(Line,pullDur);
	strcat(Line," Puls");
	strcat(Line,pushPulse);
	SSD1306_GotoXY(1, Start_Y+10);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//THIRD LINE - button functions
	if(cfgMode>0){
		strcpy(Line," Pulse ");
		strcat(Line,pushPulse);
		//strcpy(Line,"   OK   Less  More");
	}
	else{
		strcpy(Line,"Bat ");
		floatToString(floatBuf, vBat, 1, 3, 0 );
		strcat(Line,floatBuf);
		strcat(Line,"V CPU");
		floatToString(floatBuf, tCore, 1, 3, 0 );
		strcat(Line,floatBuf);
		strcat(Line,"^C"); // file font.c is upgrade to Degrees symbol on the 94(0x5E '^') char position
	}

	SSD1306_GotoXY(1, Start_Y+21);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//Draw frame around selected parameter in configuration mode
	if(cfgMode == disp_CfgModeTd_Speed){
		SSD1306_GotoXY(0, Start_Y+0);
		SSD1306_Putc('o', &Font_7x10, SSD1306_COLOR_WHITE);
	}
	else if(cfgMode == disp_CfgModeTd_Ret){
		SSD1306_GotoXY(0, Start_Y+10);
		SSD1306_Putc('o', &Font_7x10, SSD1306_COLOR_WHITE);
	}
	else if(cfgMode == disp_CfgModeTd_Pulse){
		SSD1306_GotoXY(0, Start_Y+21);
		SSD1306_Putc('o', &Font_7x10, SSD1306_COLOR_WHITE);
	}

	SSD1306_UpdateScreen();
}
/*
* Draws normal screen, cfg mode specifies which configuration is highlighted, pushspeed, pullduration is string representation of selected parameters
*/
/*
void disp_DrawScreenDate(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, RTC_DateTypeDef *rtcDate){

	SSD1306_Fill(SSD1306_COLOR_BLACK);
	//disp_DrawFrame(0, 0, 127, Start_Y+31, 0);

	//FIRST LINE
	strcpy(Line,"  Speed:   ");
	strcat(Line,pushSpeed);
	//sprintf(Line, "  Speed:   %s", pushSpeed);
	SSD1306_GotoXY(1, Start_Y+0);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//SECOND LINE
	strcpy(Line,"  Retract: ");
	strcat(Line,pullDur);
	//sprintf(Line, "  Retract: %s", pullDur);
	SSD1306_GotoXY(1, Start_Y+10);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//THIRD LINE - button functions
	if(cfgMode>0){
		strcpy(Line,"   OK   Less  More");
		//sprintf(Line, "   OK   Less  More");
	}
	else{
		strcpy(Line,"Datum: ");
		itoa(rtcDate->Date,floatBuf,10);
		strcat(Line,floatBuf);
		strcat(Line,".");
		itoa(rtcDate->Month,floatBuf,10);
		strcat(Line,floatBuf);
		strcat(Line,".");
		itoa(rtcDate->Year+2000,floatBuf,10);
		strcat(Line,floatBuf);

	}

	SSD1306_GotoXY(1, Start_Y+21);
	SSD1306_Puts(Line, &Font_7x10, SSD1306_COLOR_WHITE);

	//Draw frame around selected parameter in configuration mode
	if(cfgMode == disp_CfgModeTd_Speed){
		SSD1306_GotoXY(0, Start_Y+0);
		SSD1306_Putc('o', &Font_7x10, SSD1306_COLOR_WHITE);
	}
	else if(cfgMode == disp_CfgModeTd_Ret){
		SSD1306_GotoXY(0, Start_Y+10);
		SSD1306_Putc('o', &Font_7x10, SSD1306_COLOR_WHITE);
	}

	SSD1306_UpdateScreen();
}*/
