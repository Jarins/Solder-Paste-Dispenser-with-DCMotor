#ifndef __EEPROM_H
#define __EEPROM_H

/*
 * Thanks to Roger Clark of Melbourne and his Arduino_STM32 library
 * from which the EPROM module is used.
 * rewriteing from .cpp class to simple .c code
 * original .cpp Class:
 * https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/master/STM32F1/libraries/EEPROM
 */
#include "main.h"

// HACK ALERT. This definition may not match your processor
// To Do. Work out correct value for EEPROM_PAGE_SIZE on the STM32F103CT6 etc 
#define EEPROM_PAGE_SIZE	(uint16_t)0x400  /* Page size = 1KByte */
#define EEPROM_START_ADDRESS	((uint32_t)(0x8000000 + 64 * 1024 - 2 * EEPROM_PAGE_SIZE))

/* Pages 0 and 1 base and end addresses */
#define EEPROM_PAGE0_BASE		((uint32_t)(EEPROM_START_ADDRESS + 0x000))
#define EEPROM_PAGE1_BASE		((uint32_t)(EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE))

/* Page status definitions */
#define EEPROM_ERASED			((uint16_t)0xFFFF)	/* PAGE is empty */
#define EEPROM_RECEIVE_DATA		((uint16_t)0xEEEE)	/* PAGE is marked to receive data */
#define EEPROM_VALID_PAGE		((uint16_t)0x0000)	/* PAGE containing valid data */

#define EEPROM_DEFAULT_DATA		0xFFFF

#define IS_FLASH_ADDRESS(ADDRESS) (((ADDRESS) >= 0x08000000) && ((ADDRESS) < 0x0807FFFF))


/* Page full define */
enum xx
	{
	EEPROM_OK				= ((uint16_t)0x0000),
	EEPROM_OUT_SIZE			= ((uint16_t)0x0081),
	EEPROM_BAD_ADDRESS		= ((uint16_t)0x0082),
	EEPROM_BAD_FLASH		= ((uint16_t)0x0083),
	EEPROM_NOT_INIT			= ((uint16_t)0x0084),
	EEPROM_NO_VALID_PAGE	= ((uint16_t)0x00AB)
	};

typedef enum
	{
	FLASH_BUSY = 1,
	FLASH_ERROR_PG_,
	FLASH_ERROR_WRP_,
	FLASH_ERROR_OPT,
	FLASH_COMPLETE,
	FLASH_TIMEOUT,
	FLASH_BAD_ADDRESS
	} FLASH_Status;

//Variables

uint32_t PageBase0;
uint32_t PageBase1;
uint32_t PageSize;
uint16_t Status;

//Functions

void FLASH_Unlock(void);
void FLASH_Lock(void);

FLASH_Status FLASH_WaitForLastOperation_(uint32_t Timeout);
FLASH_Status FLASH_GetStatus(void);
FLASH_Status FLASH_ErasePage(uint32_t Page_Address);
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);

uint16_t EE_Init(void);
uint16_t EE_Format(void);
uint16_t EE_Erases(uint16_t *);
uint16_t EE_ReadAddr (uint16_t address);
uint16_t EE_Read(uint16_t address, int16_t *data);
uint16_t EE_Write(uint16_t address, int16_t data);
uint16_t EE_Count(uint16_t *);
uint16_t EE_Maxcount(void);
uint16_t EE_CheckPage(uint32_t, uint16_t);
uint16_t EE_CheckErasePage(uint32_t, uint16_t);
FLASH_Status EE_ErasePage(uint32_t);
uint32_t EE_FindValidPage(void);
uint16_t EE_GetVariablesCount(uint32_t, uint16_t);
uint16_t EE_PageTransfer(uint32_t, uint32_t, uint16_t);
uint16_t EE_VerifyPageFullWriteVariable(uint16_t, uint16_t);

#endif	/* __EEPROM_H */
