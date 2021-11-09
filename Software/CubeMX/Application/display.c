/*
 * display.c
 *
 *  Created on: Nov 27, 2019
 *      Author: Standa
 */

#include "display.h"
#include "SSD1306.h"
#include "utility.h"
//#include "qfplib.h"
//#include "bbb.h"
#include <string.h>
#include <stdio.h>
#include "gpio.h"


//INIT SCREEN INFO --- 18 char per line max
#define DEVSTRING	"Handy dispenser"
#define NAMESTRING	"By Jarin (c)2020"
#define REVSTR		"HWAA,FWAC,200929"

extern uint8_t floatToString(char * outstr, float value, uint8_t places, uint8_t minwidth);

#define TMPSIZE	21	//18 is the length of line
char Tmp[TMPSIZE];
char Line[TMPSIZE];
char floatBuf[10]; //temp buffer for float conversion

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

//sprintf () is too slow, this function is many times (hundred times) faster
uint8_t floatToString(char * outstr, float value, uint8_t places, uint8_t minwidth)
{
    // this is used to write a float value to string, outstr.  oustr is also the return value.
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
    if (value < 0)
        d *= -1.0;
    // divide by ten for each decimal place
    for (i = 0; i < places; i++)
        d/= 10.0;
    // this small addition, combined with truncation will round our values properly
    tempfloat +=  d;

    // first get value tens to be the large power of ten less than value
    if (value < 0)
        tempfloat *= -1.0;
    while ((tens * 10.0) <= tempfloat) {
        tens *= 10.0;
        tenscount += 1;
    }

    if (tenscount > 0)
        charcount += tenscount;
    else
        charcount += 1;

    if (value < 0)
        charcount += 1;
    charcount += 1 + places;

    minwidth += 1; // both count the null final character
    if (minwidth > charcount){
        extra = minwidth - charcount;
        charcount = minwidth;
    }

//    if (extra > 0 and rightjustify) {
//        for (int i = 0; i< extra; i++) {
//            outstr[c++] = ' ';
//        }
//    }

    // write out the negative if needed
    if (value < 0)
        outstr[c++] = '-';

    if (tenscount == 0)
        outstr[c++] = '0';

    for (i=0; i< tenscount; i++) {
        digit = (int) (tempfloat/tens);
        itoa(digit, &outstr[c++], 10);
        tempfloat = tempfloat - ((float)digit * tens);
        tens /= 10.0;
    }

    // if no places after decimal, stop now and return

    // otherwise, write the point and continue on
    if (places > 0)
    outstr[c++] = '.';


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
/*

 * Draws normal screen, cfg mode specifies which configuration is highlighted, pushspeed, pullduration is string representation of selected parameters
 */
void disp_DrawScreen(disp_CfgModeTd cfgMode, char* pushSpeed, char* pullDur, float vBat, float iMeas, float tCore){

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
		//strcpy(Line,"   Cfg  Pull  Push");
		strcpy(Line,"Bat: ");
		floatToString(floatBuf, vBat, 1, 3 );
		strcat(Line,floatBuf);
		strcat(Line,"V t: ");
		floatToString(floatBuf, tCore, 1, 3 );
		strcat(Line,floatBuf);

		//sprintf(Line, "Bat: %2.1fV t: %2.1fC", vBat,tCore);
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
}
