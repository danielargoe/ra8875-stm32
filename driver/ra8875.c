/*
 * ra8875.c
 *
 *  Created on: Mar 16, 2026
 *      Author: danie
 */

#include "ra8875.h"

extern SPI_HandleTypeDef hspi1;

void ra8875_initialize() {

	// hardware reset
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_Delay(10);

	// initiate pll
	ra8875_write(0x88, 0x0B);
	HAL_Delay(1);
	ra8875_write(0x89, 0x02);
	HAL_Delay(1);

	// color depth setting
    ra8875_write(0x10, 0x08);

	// set pixel clock
	ra8875_write(0x04, 0x81);
	HAL_Delay(1);

	// horizontal settings (800px wide)
	ra8875_write(0x14, 0x63);   // HDWR: (99+1)*8 = 800
	ra8875_write(0x15, 0x00);   // HNDFTR
	ra8875_write(0x16, 0x03);   // HNDR
	ra8875_write(0x17, 0x01);   // HSTR
	ra8875_write(0x18, 0x03);   // HPWR

	// vertical settings (480px tall)
	ra8875_write(0x19, 0xDF);   // VDHR0: lower byte of 479
	ra8875_write(0x1A, 0x01);   // VDHR1: upper byte of 479
	ra8875_write(0x1B, 0x0F);   // VNDR0
	ra8875_write(0x1C, 0x00);   // VNDR1
	ra8875_write(0x1D, 0x0E);   // VSTR0
	ra8875_write(0x1E, 0x00);   // VSTR1
	ra8875_write(0x1F, 0x01);   // VPWR

	// active window (HSAW, VSAW) - top left -> (HEAW, VEAW) - bottom right
	ra8875_write(0x30, 0x00); 	// HSAW0
	ra8875_write(0x31, 0x00);	// HSAW1
	ra8875_write(0x32, 0x00);	// VSAW0
	ra8875_write(0x33, 0x00);	// VSAW1
	ra8875_write(0x34, 0x1F);	// HEAW0
	ra8875_write(0x35, 0x03);	// HEAW1
	ra8875_write(0x36, 0xDF);	// VEAW0
	ra8875_write(0x37, 0x01);	// VEAW1

	// enable TFT output / GPIOX
//	ra8875_write(0xC7, 0x01);   // GPIOX on

	// config for backlight (PWM1)
	ra8875_write(0x8A, 0x8F);   // enable PWM1, choose a clock divisor
	ra8875_write(0x8B, 0xFF);   // full duty cycle

	// display on
	ra8875_write(0x01, 0x80);   // PWRR: display on, normal operation
	HAL_Delay(1);

	// backlight on
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

	// clear memory 8Eh
	ra8875_write(0x8E, 0x80);
	HAL_Delay(15);
	ra8875_write(0x8E, 0x00);
}

void ra8875_write(uint8_t reg, uint8_t data) {

	// create a buffer to hold the register and data
	uint8_t buf[2];

	buf[0] = 0x80;
	buf[1] = reg;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

	buf[0] = 0x00;
	buf[1] = data;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void ra8875_read(uint8_t reg, uint8_t* data) {

	// create a buffer to hold the register and data
	uint8_t buf[2];

	buf[0] = 0x80;
	buf[1] = reg;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

	buf[0] = 0x40;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void ra8875_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {

	// start point and end point cannot be equal
	if (x0 == x1 && y0 == y1) return;

	// set the start point of a line
	ra8875_write(0x91, x0 & 0xFF);
	ra8875_write(0x92, (x0 >> 8) & 0xFF);
	ra8875_write(0x93, y0 & 0xFF);
	ra8875_write(0x94, (y0 >> 8) & 0xFF);

	// set the end point of a line
	ra8875_write(0x95, x1 & 0xFF);
	ra8875_write(0x96, (x1 >> 8) & 0xFF);
	ra8875_write(0x97, y1 & 0xFF);
	ra8875_write(0x98, (y1 >> 8) & 0xFF);

	// set the color of a line (red, green, blue)
	ra8875_write(0x63, (color >> 11) & 0x1F);
	ra8875_write(0x64, (color >> 5) & 0x3F);
	ra8875_write(0x65, color & 0x1F);

	// set draw a line and start the drawing function
	ra8875_write(0x90, 0x08);
	ra8875_write(0x90, 0x80);

	// TODO: REFACTOR
	// infinite loop until drawing process is complete
	for (;;) {

		// read data to see if drawing is complete
		uint8_t data;
		ra8875_read(0x90, &data);

		// turn on lights on dev board to indicate that the drawing process is processing
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

		// determine if drawing function is complete
		if ((data & 0x80) == 0) break;
	}

	// turn off lights on dev board to indicate that the drawing process is complete
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

}
