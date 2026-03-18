/*
 * ra8875.h
 *
 *  Created on: Mar 16, 2026
 *      Author: danie
 */

#ifndef RA8875_H_
#define RA8875_H_

#include "stdint.h"

void ra8875_initialize();
void ra8875_write(uint8_t reg, uint8_t data);
void ra8875_read(uint8_t reg, uint8_t* data);
void ra8875_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

#endif /* RA8875_H_ */
