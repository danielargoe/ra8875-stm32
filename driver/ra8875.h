/*
 * ra8875_new.h
 *
 *  Created on: Mar 23, 2026
 *      Author: danie
 */

#ifndef RA8875_NEW_H_
#define RA8875_NEW_H_

#include "stdbool.h"
#include "stdint.h"
#include "stm32f7xx_hal.h"
#include "ra8875_regs.h"

typedef struct {
	SPI_HandleTypeDef 	*hspi;
	GPIO_TypeDef 		*scs_port;
	uint16_t			scs_pin;
	GPIO_TypeDef		*rst_port;
	uint16_t			rst_pin;

	uint8_t				pllc1;
	uint8_t				pllc2;

	uint8_t				sysr;

	uint8_t				pcsr;

	uint8_t				hdwr;
	uint8_t				hndftr;
	uint8_t				hndr;
	uint8_t				hstr;
	uint8_t				hpwr;

	uint8_t				vdhr0;
	uint8_t				vdhr1;
	uint8_t				vndr0;
	uint8_t				vndr1;
	uint8_t				vstr0;
	uint8_t				vstr1;
	uint8_t				vpwr;

	uint8_t				p1cr;
	uint8_t				p1dcr;
} ra8875_t;

void ra8875_init(ra8875_t *config);

void ra8875_set_display_power(bool enable);
void ra8875_reset_software(void);

void ra8875_set_active_window(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop);

void ra8875_set_text_mode(void);
void ra8875_set_graphic_mode(void);
void ra8875_set_memory_write_cursor_auto_increase(bool enable);

void ra8875_clear_memory(void);

void ra8875_write_text(char *text, bool font_background_transparency, uint8_t horizontal_enlargement, uint8_t vertical_enlargement, uint16_t horizontal_position, uint16_t vertical_position, uint16_t color_background, uint16_t color_foreground);

void ra8875_draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t color);
void ra8875_draw_triangle(uint16_t x_start, uint16_t y_start, uint16_t x_middle, uint16_t y_middle, uint16_t x_stop, uint16_t y_stop, uint16_t color, bool fill);
void ra8875_draw_rectangle(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t color, bool fill);
void ra8875_draw_rectangle_circle(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t axis_long, uint16_t axis_short, uint16_t color, bool fill);
void ra8875_draw_circle(uint16_t x_center, uint16_t y_center, uint8_t radius, uint16_t color, bool fill);
void ra8875_draw_ellipse(uint16_t x_center, uint16_t y_center, uint16_t axis_long, uint16_t axis_short, uint16_t color, bool fill);
void ra8875_draw_ellipse_curve(uint16_t x_center, uint16_t y_center, uint16_t axis_long, uint16_t axis_short, uint16_t color, uint8_t draw_curve_part_select, bool fill);

#endif /* RA8875_NEW_H_ */
