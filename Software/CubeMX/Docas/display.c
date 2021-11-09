/*
 * display.c
 *
 *  Created on: Nov 27, 2019
 *      Author: Standa
 */

#include "display.h"
#include "ssd1306.h"
#include "utility.h"
#include "qfplib.h"
#include "bbb.h"
#include <string.h>
#include <stdio.h>


//INIT SCREEN INFO --- 18 char per line max
#define DEVSTRING	"Handy dispenser"
#define NAMESTRING	"By Stanislav Subrt"
#define REVSTR		"HWAA, FWAC, 200929"


#define TMPSIZE	32	//18 is the length of line
char Tmp[TMPSIZE];
char Line[TMPSIZE];


/*
 * Draws frame around specified area (optionally filled with black color)
 */
void disp_DrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t isFilled){

	for(uint8_t i=y1; i<=y2; i++){
		if(isFilled) for(uint8_t j=x1; j<=x2; j++) ssd1306_DrawPixel(j, i, Black);
		ssd1306_DrawPixel(x1, i, White);
		ssd1306_DrawPixel(x2, i, White);
	}

	for(uint8_t i=x1; i<=x2; i++){
		ssd1306_DrawPixel(i, y1, White);
		ssd1306_DrawPixel(i, y2, White);
	}
}


/*
 * Draws "saved" pop-up window in the center of the screen
 */
void disp_DrawSavedPopup(void){

	  //SECOND LINE
	strcpy(Line + 0, "      Saved!      ");
	disp_DrawFrame(26, 8, 100, 22, 1);
	ssd1306_SetCursor(28, 10);
	ssd1306_WriteString(Line, Font_7x10, White);
	ssd1306_UpdateScreen();
}


/*
 * Draws loading screen with battery voltage and core temperature
 */
void disp_DrawInitScreen(void){
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(DEVSTRING, Font_7x10, White);

	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString(NAMESTRING, Font_7x10, White);

	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString(REVSTR, Font_7x10, White);
	ssd1306_UpdateScreen();
}


/*
 * Draws normal screen, cfg mode specifies which configuration is highlighted, pushspeed, pullduration is string representation of selected parameters
 */
void disp_DrawScreen(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, float vBat, float iMeas, float tCore){
	ssd1306_Fill(Black);

	//FIRST LINE
	sprintf(Line, "  Speed:   %s", pushSpeed);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(Line, Font_7x10, White);

	//SECOND LINE
	sprintf(Line, "  Retract: %s", pullDur);
	ssd1306_SetCursor(0, 11);
	ssd1306_WriteString(Line, Font_7x10, White);

	//THIRD LINE - button functions
	if(cfgMode>0){
		sprintf(Line, "   OK   Less  More");
	}
	else{
		sprintf(Line, "   Cfg  Pull  Push");
	}

	ssd1306_SetCursor(0, 21);
	ssd1306_WriteString(Line, Font_7x10, White);

	//Draw frame around selected parameter in configuration mode
	if(cfgMode == disp_CfgModeTd_Speed){
		ssd1306_SetCursor(0, 0);
		ssd1306_WriteChar('o', Font_7x10, White);
	}
	else if(cfgMode == disp_CfgModeTd_Ret){
		ssd1306_SetCursor(0, 11);
		ssd1306_WriteChar('o', Font_7x10, White);
	}

	ssd1306_UpdateScreen();
}
