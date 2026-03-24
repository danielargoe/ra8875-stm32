/*
 * ra8875_new.c
 *
 *  Created on: Mar 23, 2026
 *      Author: danie
 */

#include "stdbool.h"
#include "stdint.h"
#include "ra8875.h"

static ra8875_t ra8875_config;

static void ra8875_delay_ms(uint32_t delay);
static void ra8875_write_scs_low(void);
static void ra8875_write_scs_high(void);

static void ra8875_write_data(uint8_t data);
static void ra8875_read_data(uint8_t *data);
static void ra8875_write_command(uint8_t reg);
static void ra8875_read_status(uint8_t *status);

static void ra8875_write_register(uint8_t reg, uint8_t data);
static void ra8875_read_register(uint8_t reg, uint8_t *data);
static void ra8875_read_status_register(uint8_t *data);

static void ra8875_update_register_bits(uint8_t reg, uint8_t mask, uint8_t data);

static void ra8875_init_phase_locked_loop(void);
static void ra8875_init_system_configuration(void);
static void ra8875_init_pixel_clock(void);
static void ra8875_init_horizontal_settings(void);
static void ra8875_init_vertical_settings(void);
static void ra8875_init_pulse_width_modulation_1(void);

static void ra8875_write_memory_data(uint8_t data);
//static void ra8875_read_memory_data(uint8_t *data);

static void ra8875_write_text_busy_wait(void);
static void ra8875_clear_memory_busy_wait(void);
static void ra8875_draw_shape_busy_wait(uint8_t reg, uint8_t mask);

static void ra8875_set_font_background_transparency(bool enable);
static void ra8875_set_font_horizontal_enlargement(uint8_t enlargement);
static void ra8875_set_font_vertical_enlargement(uint8_t enlargement);
static void ra8875_set_font_cursor_horizontal_position(uint16_t position);
static void ra8875_set_font_cursor_vertical_position(uint16_t position);

static void ra8875_set_color_background(uint16_t color);
static void ra8875_set_color_foreground(uint16_t color);

static void ra8875_draw_horizontal_position_start(uint16_t position);
static void ra8875_draw_vertical_position_start(uint16_t position);
static void ra8875_draw_horizontal_position_stop(uint16_t position);
static void ra8875_draw_vertical_position_stop(uint16_t position);
static void ra8875_draw_horizontal_position_middle(uint16_t position);
static void ra8875_draw_vertical_position_middle(uint16_t position);
static void ra8875_draw_horizontal_position_center_circle(uint16_t position);static void ra8875_draw_vertical_position_center_circle(uint16_t position);
static void ra8875_draw_horizontal_position_center_ellipse(uint16_t position);
static void ra8875_draw_vertical_position_center_ellipse(uint16_t position);
static void ra8875_draw_radius(uint8_t radius);
static void ra8875_draw_curve_part_select(uint8_t curve_part_select);
static void ra8875_draw_axis_long(uint16_t axis);
static void ra8875_draw_axis_short(uint16_t axis);


static void ra8875_delay_ms(uint32_t delay) {

	// set a delay
	HAL_Delay(delay);
}
static void ra8875_write_scs_low(void) {

	// set spi chip select low
	HAL_GPIO_WritePin(ra8875_config.scs_port, ra8875_config.scs_pin, GPIO_PIN_RESET);
}
static void ra8875_write_scs_high(void) {

	// set spi chip select high
	HAL_GPIO_WritePin(ra8875_config.scs_port, ra8875_config.scs_pin, GPIO_PIN_SET);
}

static void ra8875_write_data(uint8_t data) {

	// write data to a register
	uint8_t tx[2];
	tx[0] = 0x00;
	tx[1] = data;
	ra8875_write_scs_low();
	HAL_SPI_Transmit(ra8875_config.hspi, tx, 2, HAL_MAX_DELAY);
	ra8875_write_scs_high();
}
static void ra8875_read_data(uint8_t *data) {

	// read data from a register
	uint8_t tx[2];
	uint8_t rx[2];
	tx[0] = 0x40;
	tx[1] = 0x00;
	rx[0] = 0x00;
	rx[1] = 0x00;
	ra8875_write_scs_low();
	HAL_SPI_TransmitReceive(ra8875_config.hspi, tx, rx, 2, HAL_MAX_DELAY);
	ra8875_write_scs_high();
	*data = rx[1];
}
static void ra8875_write_command(uint8_t reg) {

	// write command to a register
	uint8_t tx[2];
	tx[0] = 0x80;
	tx[1] = reg;
	ra8875_write_scs_low();
	HAL_SPI_Transmit(ra8875_config.hspi, tx, 2, HAL_MAX_DELAY);
	ra8875_write_scs_high();
}
static void ra8875_read_status(uint8_t *status) {

	// read status from a register
	uint8_t tx[2];
	uint8_t rx[2];
	tx[0] = 0xC0;
	tx[1] = 0x00;
	rx[0] = 0x00;
	rx[1] = 0x00;
	ra8875_write_scs_low();
	HAL_SPI_TransmitReceive(ra8875_config.hspi, tx, rx, 2, HAL_MAX_DELAY);
	ra8875_write_scs_high();
	*status = rx[1];
}

static void ra8875_write_register(uint8_t reg, uint8_t data) {

	// write data to a register given the register and data
	ra8875_write_command(reg);
	ra8875_write_data(data);
}
static void ra8875_read_register(uint8_t reg, uint8_t *data) {

	// read data to a memory address given the register
	ra8875_write_command(reg);
	ra8875_read_data(data);
}
static void ra8875_read_status_register(uint8_t *data) {

	// read data to a memory address from the status register
	ra8875_read_status(data);
}

static void ra8875_update_register_bits(uint8_t reg, uint8_t mask, uint8_t data) {

	// read data from register and then update the value based on the mask and data
	uint8_t current;
	ra8875_read_register(reg, &current);

	current = (current & ~mask) | (data & mask);
	ra8875_write_register(reg, current);
}

static void ra8875_init_phase_locked_loop(void) {

	// phased lock loop init
	ra8875_write_register(0x88, ra8875_config.pllc1);
	ra8875_delay_ms(1);
	ra8875_write_register(0x89, ra8875_config.pllc2);
	ra8875_delay_ms(1);
}
static void ra8875_init_system_configuration(void) {

	// system configuration init (color depth and mcu interface)
	ra8875_write_register(0x10, ra8875_config.sysr);
}
static void ra8875_init_pixel_clock(void) {

	// pixel clock init
	ra8875_write_register(0x04, ra8875_config.pcsr);
	ra8875_delay_ms(1);
}
static void ra8875_init_horizontal_settings(void) {

	// horizontal settings init
	ra8875_write_register(0x14, ra8875_config.hdwr);
	ra8875_write_register(0x15, ra8875_config.hndftr);
	ra8875_write_register(0x16, ra8875_config.hndr);
	ra8875_write_register(0x17, ra8875_config.hstr);
	ra8875_write_register(0x18, ra8875_config.hpwr);
}
static void ra8875_init_vertical_settings(void) {

	// vertical settings init
	ra8875_write_register(0x19, ra8875_config.vdhr0);
	ra8875_write_register(0x1A, ra8875_config.vdhr1);
	ra8875_write_register(0x1B, ra8875_config.vndr0);
	ra8875_write_register(0x1C, ra8875_config.vndr1);
	ra8875_write_register(0x1D, ra8875_config.vstr0);
	ra8875_write_register(0x1E, ra8875_config.vstr1);
	ra8875_write_register(0x1F, ra8875_config.vpwr);
}
static void ra8875_init_pulse_width_modulation_1(void) {

	// pulse width modulation 1 init
	ra8875_write_register(0x8A, ra8875_config.p1cr);
	ra8875_write_register(0x8B, ra8875_config.p1dcr);
}

static void ra8875_write_memory_data(uint8_t data) {

	// write data to memory
	ra8875_write_register(0x02, data);

	// loop to check if memory is being written
	ra8875_write_text_busy_wait();
}
//static void ra8875_read_memory_data(uint8_t *data) {
//
//	// read data from memory
//	ra8875_read_register(0x02, data);
//}

static void ra8875_write_text_busy_wait(void) {

	// loop to check if memory is being written (status register)
	for (;;) {
		uint8_t status;
		ra8875_read_status_register(&status);

		if ((status & 0x80) == 0u) break;
	}
}
static void ra8875_clear_memory_busy_wait(void) {

	// loop to check if memory is being cleared
	for (;;) {
		uint8_t status;
		ra8875_read_register(0x8E, &status);

		if ((status & 0x80) == 0u) break;
	}
}
static void ra8875_draw_shape_busy_wait(uint8_t reg, uint8_t mask) {

	// loop to check if memory is being cleared
	for (;;) {
		uint8_t status;
		ra8875_read_register(reg, &status);

		if ((status & mask) == 0u) break;
	}
}

static void ra8875_set_font_background_transparency(bool enable) {

	// set font transparency
	if (enable) {
		ra8875_update_register_bits(0x22, 0x40, 0x40);
	} else {
		ra8875_update_register_bits(0x22, 0x40, 0x00);
	}
}
static void ra8875_set_font_horizontal_enlargement(uint8_t enlargement) {

	// check to see if parameter is valid
	if (enlargement < 1u) {
		enlargement = 1;
	} else if (enlargement > 4u) {
		enlargement = 4;
	}

	// subtract one to get the valid bits [0-3]
	uint8_t data = (uint8_t)(enlargement - 1u);

	ra8875_update_register_bits(0x22, 0x0Cu, (uint8_t)(data << 2));
}
static void ra8875_set_font_vertical_enlargement(uint8_t enlargement) {

	// check to see if parameter is valid
	if (enlargement < 1u) {
		enlargement = 1;
	} else if (enlargement > 4u) {
		enlargement = 4;
	}

	// subtract one to get the valid bits [0-3]
	uint8_t data = (uint8_t)(enlargement - 1u);

	ra8875_update_register_bits(0x22, 0x03, data);
}
static void ra8875_set_font_cursor_horizontal_position(uint16_t position) {

	// set font cursor horizontal position
	ra8875_write_register(0x2A, position & 0xFF);
	ra8875_write_register(0x2B, (position >> 8) & 0xFF);
}
static void ra8875_set_font_cursor_vertical_position(uint16_t position) {

	// set font cursor vertical position
	ra8875_write_register(0x2C, position & 0xFF);
	ra8875_write_register(0x2D, (position >> 8) & 0xFF);
}

static void ra8875_set_color_background(uint16_t color) {

	// determine if bpp is 8 or 16
	uint8_t data;
	ra8875_read_register(0x10, &data);

	// set background color
	(data & 0x08) ? ra8875_write_register(0x60, (color >> 11) & 0x1F): ra8875_write_register(0x60, (color >> 5) & 0x07);
	(data & 0x08) ? ra8875_write_register(0x61, (color >> 05) & 0x3F): ra8875_write_register(0x61, (color >> 2) & 0x07);
	(data & 0x08) ? ra8875_write_register(0x62, (color >> 00) & 0x1F): ra8875_write_register(0x62, (color >> 0) & 0x03);
}
static void ra8875_set_color_foreground(uint16_t color) {

	// determine if bpp is 8 or 16
	uint8_t data;
	ra8875_read_register(0x10, &data);

	// set background color
	(data & 0x08) ? ra8875_write_register(0x63, (color >> 11) & 0x1F): ra8875_write_register(0x63, (color >> 5) & 0x07);
	(data & 0x08) ? ra8875_write_register(0x64, (color >> 05) & 0x3F): ra8875_write_register(0x64, (color >> 2) & 0x07);
	(data & 0x08) ? ra8875_write_register(0x65, (color >> 00) & 0x1F): ra8875_write_register(0x65, (color >> 0) & 0x03);
}

static void ra8875_draw_horizontal_position_start(uint16_t position) {

	// set horizontal position start
	ra8875_write_register(0x91, position & 0xFF);
	ra8875_write_register(0x92, (position >> 8) & 0xFF);
}
static void ra8875_draw_vertical_position_start(uint16_t position) {

	// set vertical position start
	ra8875_write_register(0x93, position & 0xFF);
	ra8875_write_register(0x94, (position >> 8) & 0xFF);
}
static void ra8875_draw_horizontal_position_stop(uint16_t position) {

	// set horizontal position stop
	ra8875_write_register(0x95, position & 0xFF);
	ra8875_write_register(0x96, (position >> 8) & 0xFF);
}
static void ra8875_draw_vertical_position_stop(uint16_t position) {

	// set vertical position stop
	ra8875_write_register(0x97, position & 0xFF);
	ra8875_write_register(0x98, (position >> 8) & 0xFF);
}
static void ra8875_draw_horizontal_position_middle(uint16_t position) {

	// set horizontal position middle
	ra8875_write_register(0xA9, position & 0xFF);
	ra8875_write_register(0xAA, (position >> 8) & 0xFF);
}
static void ra8875_draw_vertical_position_middle(uint16_t position) {

	// set vertical position middle
	ra8875_write_register(0xAB, position & 0xFF);
	ra8875_write_register(0xAC, (position >> 8) & 0xFF);
}
static void ra8875_draw_horizontal_position_center_circle(uint16_t position) {

	// set horizontal position center
	ra8875_write_register(0x99, position & 0xFF);
	ra8875_write_register(0x9A, (position >> 8) & 0xFF);
}
static void ra8875_draw_vertical_position_center_circle(uint16_t position) {

	// set vertical position center
	ra8875_write_register(0x9B, position & 0xFF);
	ra8875_write_register(0x9C, (position >> 8) & 0xFF);
}
static void ra8875_draw_radius(uint8_t radius) {

	// set radius
	ra8875_write_register(0x9D, radius & 0xFF);
}
static void ra8875_draw_curve_part_select(uint8_t curve_part_select) {

	// set draw curve part select
	ra8875_update_register_bits(0xA0, 0x03, curve_part_select);
}
static void ra8875_draw_axis_long(uint16_t axis) {

	// set axis long
	ra8875_write_register(0xA1, axis & 0xFF);
	ra8875_write_register(0xA2, (axis >> 8) & 0xFF);
}
static void ra8875_draw_axis_short(uint16_t axis) {

	// set axis short
	ra8875_write_register(0xA3, axis & 0xFF);
	ra8875_write_register(0xA4, (axis >> 8) & 0xFF);
}
static void ra8875_draw_horizontal_position_center_ellipse(uint16_t position) {

	// set horizontal position center
	ra8875_write_register(0xA5, position & 0xFF);
	ra8875_write_register(0xA6, (position >> 8) & 0xFF);
}
static void ra8875_draw_vertical_position_center_ellipse(uint16_t position) {

	// set vertical position center
	ra8875_write_register(0xA7, position & 0xFF);
	ra8875_write_register(0xA8, (position >> 8) & 0xFF);
}

void ra8875_init(ra8875_t *config) {

	// set config
	ra8875_config = *config;

	// set the ra8875 configuration settings
	ra8875_reset_software();
	ra8875_init_phase_locked_loop();
	ra8875_init_system_configuration();
	ra8875_init_pixel_clock();
	ra8875_init_horizontal_settings();
	ra8875_init_vertical_settings();
	ra8875_set_active_window(0, 0, 799, 479);
	ra8875_init_pulse_width_modulation_1();

	// turn display on
	ra8875_set_display_power(true);

	// clear memory
	ra8875_clear_memory();
}

void ra8875_set_display_power(bool enable) {

	// turn power on/off
	if (enable) {
		ra8875_update_register_bits(0x01, 0x80, 0x80);
	} else {
		ra8875_update_register_bits(0x01, 0x80, 0x00);
	}
}
void ra8875_reset_software(void) {

	// reset software
	HAL_GPIO_WritePin(ra8875_config.rst_port, ra8875_config.rst_pin, GPIO_PIN_SET);
	ra8875_delay_ms(10);
	HAL_GPIO_WritePin(ra8875_config.rst_port, ra8875_config.rst_pin, GPIO_PIN_RESET);
	ra8875_delay_ms(10);
	HAL_GPIO_WritePin(ra8875_config.rst_port, ra8875_config.rst_pin, GPIO_PIN_SET);
	ra8875_delay_ms(10);
}

void ra8875_set_active_window(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop) {

	// set active window
	ra8875_write_register(0x30, x_start & 0xFF);
	ra8875_write_register(0x31, (x_start >> 8) & 0xFF);
	ra8875_write_register(0x32, y_start & 0xFF);
	ra8875_write_register(0x33, (y_start >> 8) & 0xFF);
	ra8875_write_register(0x34, x_stop & 0xFF);
	ra8875_write_register(0x35, (x_stop >> 8) & 0xFF);
	ra8875_write_register(0x36, y_stop & 0xFF);
	ra8875_write_register(0x37, (y_stop >> 8) & 0xFF);
}

void ra8875_set_text_mode(void) {

	// set text mode
	ra8875_update_register_bits(0x40, 0x80, 0x80);
}
void ra8875_set_graphic_mode(void) {

	// set graphic mode
	ra8875_update_register_bits(0x40, 0x80, 0x00);
}
void ra8875_set_memory_write_cursor_auto_increase(bool enable) {

	// set graphic mode
	if (enable) {
		ra8875_update_register_bits(0x40, 0x02, 0x02);
	} else {
		ra8875_update_register_bits(0x40, 0x02, 0x00);
	}
}

void ra8875_clear_memory(void) {

	// clear memory
	ra8875_update_register_bits(0x8E, 0x80, 0x80);

	// loop to check if memory is still being cleared
	ra8875_clear_memory_busy_wait();
}

void ra8875_write_text(char *text, uint16_t horizontal_position, uint16_t vertical_position, uint8_t horizontal_enlargement, uint8_t vertical_enlargement, uint16_t color_background, uint16_t color_foreground, bool background_transparency) {

	// needs checks later...

	// reset font settings
	ra8875_write_register(0x22, 0x00);

	// set font settings
	ra8875_set_font_cursor_horizontal_position(horizontal_position);
	ra8875_set_font_cursor_vertical_position(vertical_position);
	ra8875_set_font_horizontal_enlargement(horizontal_enlargement);
	ra8875_set_font_vertical_enlargement(vertical_enlargement);
	ra8875_set_color_background(color_background);
	ra8875_set_color_foreground(color_foreground);
	ra8875_set_font_background_transparency(background_transparency);

	// check to see if cursor auto increase is on, and if not, we manually increment the text
	uint8_t auto_increase;
	ra8875_read_register(0x40, &auto_increase);

	if ((auto_increase & 0x02) == 0u) {

		// iterate through text and display it with the offset provided from horizontal_enlargement
		for (size_t i = 0; text[i] != '\0'; i++) {

			// set horizontal font cursor position
			ra8875_set_font_cursor_horizontal_position(horizontal_position);

			// write character to memory
			ra8875_write_memory_data(text[i]);

			// increase offset
			horizontal_position += (uint16_t)(horizontal_enlargement * 8u);
		}
	} else {

		// iterate through text and display it
		for (size_t i = 0; text[i] != '\0'; i++) {

			// write character to memory
			ra8875_write_memory_data(text[i]);
		}

	}
}

void ra8875_draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t color) {

	// reset draw controls
	ra8875_write_register(0x90, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_start(x_start);
	ra8875_draw_vertical_position_start(y_start);
	ra8875_draw_horizontal_position_stop(x_stop);
	ra8875_draw_vertical_position_stop(y_stop);
	ra8875_set_color_foreground(color);

	// set signal
	ra8875_update_register_bits(0x90, 0x10, 0x00);

	// draw line
	ra8875_update_register_bits(0x90, 0x80, 0x80);

	// check if drawing is complete
	ra8875_draw_shape_busy_wait(0x90, 0x80);
}
void ra8875_draw_triangle(uint16_t x_start, uint16_t y_start, uint16_t x_middle, uint16_t y_middle, uint16_t x_stop, uint16_t y_stop, uint16_t color, bool fill) {

	// reset draw controls
	ra8875_write_register(0x90, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_start(x_start);
	ra8875_draw_vertical_position_start(y_start);
	ra8875_draw_horizontal_position_middle(x_middle);
	ra8875_draw_vertical_position_middle(y_middle);
	ra8875_draw_horizontal_position_stop(x_stop);
	ra8875_draw_vertical_position_stop(y_stop);
	ra8875_set_color_foreground(color);

	// set fill
	(fill) ? ra8875_update_register_bits(0x90, 0x20, 0x20) : ra8875_update_register_bits(0x90, 0x20, 0x00);

	// set signal
	ra8875_update_register_bits(0x90, 0x01, 0x01);

	// draw triangle
	ra8875_update_register_bits(0x90, 0x80, 0x80);

	// check if drawing is complete
	ra8875_draw_shape_busy_wait(0x90, 0x80);
}
void ra8875_draw_rectangle(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t color, bool fill) {

	// reset draw controls
	ra8875_write_register(0x90, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_start(x_start);
	ra8875_draw_vertical_position_start(y_start);
	ra8875_draw_horizontal_position_stop(x_stop);
	ra8875_draw_vertical_position_stop(y_stop);
	ra8875_set_color_foreground(color);

	// set fill
	(fill) ? ra8875_update_register_bits(0x90, 0x20, 0x20) : ra8875_update_register_bits(0x90, 0x20, 0x00);

	// set signal
	ra8875_update_register_bits(0x90, 0x10, 0x10);

	// draw rectangle
	ra8875_update_register_bits(0x90, 0x80, 0x80);

	// check if drawing is complete
	ra8875_draw_shape_busy_wait(0x90, 0x80);
}
void ra8875_draw_rectangle_circle(uint16_t x_start, uint16_t y_start, uint16_t x_stop, uint16_t y_stop, uint16_t axis_long, uint16_t axis_short, uint16_t color, bool fill) {

	// reset draw controls
	ra8875_write_register(0xA0, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_start(x_start);
	ra8875_draw_vertical_position_start(y_start);
	ra8875_draw_horizontal_position_stop(x_stop);
	ra8875_draw_vertical_position_stop(y_stop);
	ra8875_draw_axis_long(axis_long);
	ra8875_draw_axis_short(axis_short);
	ra8875_set_color_foreground(color);

	// set fill
	(fill) ? ra8875_update_register_bits(0xA0, 0x40, 0x40): ra8875_update_register_bits(0xA0, 0x40, 0x00);

	// set signal
	ra8875_update_register_bits(0xA0, 0x20, 0x20);

	// draw rectangle circle
	ra8875_update_register_bits(0xA0, 0x80, 0x80);

	// check if drawing is complete
	ra8875_draw_shape_busy_wait(0xA0, 0x80);
}
void ra8875_draw_circle(uint16_t x_center, uint16_t y_center, uint8_t radius, uint16_t color, bool fill) {

	// reset draw controls
	ra8875_write_register(0x90, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_center_circle(x_center);
	ra8875_draw_vertical_position_center_circle(y_center);
	ra8875_draw_radius(radius);
	ra8875_set_color_foreground(color);

	// set fill
	(fill) ? ra8875_update_register_bits(0x90, 0x20, 0x20) : ra8875_update_register_bits(0x90, 0x20, 0x00);

	// draw circle
	ra8875_update_register_bits(0x90, 0x40, 0x40);

	// check if drawing is completed
	ra8875_draw_shape_busy_wait(0x90, 0x40);
}
void ra8875_draw_ellipse(uint16_t x_center, uint16_t y_center, uint16_t axis_long, uint16_t axis_short, uint16_t color, bool fill) {

	// reset draw controls
	ra8875_write_register(0xA0, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_center_ellipse(x_center);
	ra8875_draw_vertical_position_center_ellipse(y_center);
	ra8875_draw_axis_long(axis_long);
	ra8875_draw_axis_short(axis_short);
	ra8875_set_color_foreground(color);

	// set fill
	(fill) ? ra8875_update_register_bits(0xA0, 0x40, 0x40): ra8875_update_register_bits(0xA0, 0x40, 0x00);

	// set signals
	ra8875_update_register_bits(0xA0, 0x20, 0x00);
	ra8875_update_register_bits(0xA0, 0x10, 0x00);

	// draw ellipse
	ra8875_update_register_bits(0xA0, 0x80, 0x80);

	// check if drawing is completed
	ra8875_draw_shape_busy_wait(0xA0, 0x80);
}
void ra8875_draw_ellipse_curve(uint16_t x_center, uint16_t y_center, uint16_t axis_long, uint16_t axis_short, uint16_t color, uint8_t curve_part_select, bool fill) {

	// reset draw controls
	ra8875_write_register(0xA0, 0x00);

	// set draw settings
	ra8875_draw_horizontal_position_center_ellipse(x_center);
	ra8875_draw_vertical_position_center_ellipse(y_center);
	ra8875_draw_axis_long(axis_long);
	ra8875_draw_axis_short(axis_short);
	ra8875_set_color_foreground(color);
	ra8875_draw_curve_part_select(curve_part_select);

	// set fill
	(fill) ? ra8875_update_register_bits(0xA0, 0x40, 0x40): ra8875_update_register_bits(0xA0, 0x40, 0x00);

	// set signals
	ra8875_update_register_bits(0xA0, 0x20, 0x00);
	ra8875_update_register_bits(0xA0, 0x10, 0x10);

	// draw ellipse curve
	ra8875_update_register_bits(0xA0, 0x80, 0x80);

	// check if drawing is completed
	ra8875_draw_shape_busy_wait(0xA0, 0x80);
}
