/*
 * test.h
 *
 *  Created on: 17-Jun-2019
 *      Author: poe
 */

#ifndef TEST_H_
#define TEST_H_

#include "stdint.h"

void TestLines (uint8_t color);

void TestRectangles (uint8_t color);

void TestFilledRectangles (uint8_t color);

void TestFilledCircles(uint8_t radius, uint16_t color);

void TestCircles(uint8_t radius, uint16_t color);

void TestTriangles(uint8_t color);

#endif /* TEST_H_ */
