/*
 * bbb.h
 *
 *  Created on: Nov 25, 2019
 *      Author: Standa
 */

#ifndef BBB_H_
#define BBB_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"


typedef enum{
	bbb_EvtTd_Low = -2,
	bbb_EvtTd_High = -1,
	bbb_EvtTd_None = 0,
	bbb_EvtTd_Pressed,
	bbb_EvtTd_Released,
	bbb_EvtTd_SingleReleased,
	bbb_EvtTd_DoubleReleased,
	bbb_EvtTd_LongReleased,
}bbb_EvtTd;


typedef struct{
	GPIO_TypeDef* Port;
	uint32_t Pin;
	uint32_t LastState;

	uint32_t ReleasedTimestamp;
	uint32_t StateChangeInterval;
	uint32_t SingleReleasedEnsuredTimestamp;
	uint32_t PrevBtnStateTimestamp;
	bbb_EvtTd PrevBtnEvt;
	uint8_t PrevBtnState;
}bbb_BtnTd;


typedef enum{
	bbb_ToneTd_None = 0,
	bbb_ToneTd_Low = 1,
	bbb_ToneTd_Med = 2,
	bbb_ToneTd_High = 3,
	bbb_ToneTd_VeryHigh = 4
}bbb_ToneTd;


extern void bbb_Init(void);
extern uint8_t bbb_Update(void);

extern bbb_BtnTd* bbb_GetPushBtnState(void);
extern bbb_BtnTd* bbb_GetPullBtnState(void);
extern bbb_BtnTd* bbb_GetCfgBtnState(void);

extern void bbb_WaitSync(uint16_t waitMs);
extern void bbb_WaitSyncCond(uint16_t waitMs, uint8_t (*stopRequestFn)(void));
extern void bbb_GetBtnStates(uint8_t* pushBtn, uint8_t* pullBtn, uint8_t *cfgBtn);

extern void bbb_Beep(uint8_t tone, uint8_t time);
extern void bbb_Melody2(uint8_t tone1, uint8_t tone2, uint16_t time);
extern void bbb_Melody3(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint16_t time);
extern void bbb_Melody4(uint8_t tone1, uint8_t tone2, uint8_t tone3, uint8_t tone4, uint16_t time);

#endif /* BBB_H_ */
