/*
 * ButtonBeep.c
 *
 *  Created on: Nov 25, 2019
 *      Author: Standa
 *      reprogrammed Nov 22, 2021
 *      Author Jarin
 */

#ifndef BUTTONBEEP_H_
#define BUTTONBEEP_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"


enum Buttons {
	PUSH = 0,
	PULL,
	CFG,
	CountButtons // is not in the set of active elements, only for element count in ENUM
};

typedef enum{
	But_None = 0,
	But_Pressed,
	But_Released,
	//EvtTd_LongReleased,
}BtnEvtTd;

typedef struct{
	bool IntStart;
	uint8_t LastState;
	GPIO_TypeDef* Port;
	uint16_t Pin;
}BtnState;

typedef enum{
	bbb_ToneTd_None = 0,
	bbb_ToneTd_Low = 1,
	bbb_ToneTd_Med = 2,
	bbb_ToneTd_High = 3,
	bbb_ToneTd_VeryHigh = 4
}bbb_ToneTd;

extern volatile BtnState* ButList[CountButtons];

extern void bbb_Beep(uint8_t tone, uint8_t time);
extern void bbb_Melody2(uint8_t tone1, uint8_t tone2, uint16_t time);
extern void bbb_Melody3(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint16_t time);
extern void bbb_Melody4(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint8_t tone4, uint16_t time);

#endif /* BUTTONBEEP_H_ */
