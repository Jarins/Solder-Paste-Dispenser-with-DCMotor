/*
 * error.h
 *
 *  Created on: 11 Nov 2017
 *      Author: Standa
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <stdint.h>


/*
 * Error enumeration to be used in all functions returning error states.
 * Values might be added, but never removed. Only positive values shall be specified for each enumerated error.
 */
typedef enum{
	err_Td_Ok = 0,
	err_Td_General = 1,
	err_Td_Param = 2,
	err_Td_Ack = 3,
	err_Td_Nack = 4,
	err_Td_Init = 5,
	err_Td_Busy = 6,
	err_Td_Timeout = 7,
	err_Td_Satur = 8,
	err_Td_Range = 9,
	err_Td_NotExist = 10,
	err_Td_Null = 11,
	err_Td_NotImple = 12,
	err_Td_NotFound = 13,
	err_Td_NotSup = 14,
	err_Td_NotValid = 15,
	err_Td_Overflow = 16,
	err_Td_CRC = 17,
	err_Td_Disabled = 18,
} err_Td;


extern void err_Init(void);
extern err_Td err_Add(err_Td err, uint32_t info);
extern err_Td err_Read(uint32_t *timeStamp, err_Td *err, uint32_t *info);
extern err_Td err_PeekLast(uint32_t *timeStamp, err_Td *err, uint32_t *info);
extern err_Td err_Update(void);

#endif /* ERROR_H_ */
