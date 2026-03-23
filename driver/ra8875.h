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
uint8_t ra8875_read(uint8_t reg);
uint8_t ra8875_read_status();
void ra8875_set_color(uint16_t color);
void ra8875_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ra8875_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t fill);
void ra8875_draw_square(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint8_t fill);
void ra8875_draw_square_circle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t a_l, uint16_t a_s, uint16_t color, uint8_t fill);
void ra8875_draw_circle(uint16_t x0, uint16_t y0, uint8_t radius, uint16_t color, uint8_t fill);
void ra8875_draw_ellipse(uint16_t x0, uint16_t y0, uint16_t a_l, uint16_t a_s, uint16_t color, uint8_t fill);
void ra8875_draw_ellipse_curve(uint16_t x0, uint16_t y0, uint16_t a_l, uint16_t a_s, uint16_t color, uint8_t fill, uint8_t part);
uint8_t ra8875_wait_while_busy(uint8_t reg, uint8_t mask, uint32_t timeout_ms);
uint8_t ra8875_wait_status_while_busy(uint8_t mask, uint32_t timeout_ms);
void ra8875_write_text(char* s, uint16_t x, uint16_t y, uint8_t size, uint16_t color);

#endif /* RA8875_H_ */
