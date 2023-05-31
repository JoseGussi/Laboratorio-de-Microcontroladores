/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Chuck McManis <cmcmanis@mcmanis.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/usart.h> 
#include <stdint.h>
#include <math.h>
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"
#include <stdio.h>


//Defines
#define GYR_RNW			(1 << 7) /* Write when zero  (ahorita en 1, read)*/  
#define GYR_MNS			(1 << 6) /* Multiple reads when 1 */
#define GYR_WHO_AM_I		0x0F
#define GYR_OUT_TEMP		0x26
#define GYR_STATUS_REG		0x27
#define GYR_CTRL_REG1		0x20
#define GYR_CTRL_REG1_PD	(1 << 3)
#define GYR_CTRL_REG1_XEN	(1 << 1)
#define GYR_CTRL_REG1_YEN	(1 << 0)
#define GYR_CTRL_REG1_ZEN	(1 << 2)
#define GYR_CTRL_REG1_BW_SHIFT	4
#define GYR_CTRL_REG4		0x23
#define GYR_CTRL_REG4_FS_SHIFT	4
#define GYR_OUT_X_L		0x28
#define GYR_OUT_X_H		0x29
#define GYR_OUT_Y_L		0x2A
#define GYR_OUT_Y_H		0x2B
#define GYR_OUT_Z_L		0x2C
#define GYR_OUT_Z_H		0x2D
#define L3GD20_SENSITIVITY_250DPS  (0.00875F)      
#define L3GD20_SENSITIVITY_500DPS  (0.0175F)       
#define L3GD20_SENSITIVITY_2000DPS (0.070F)        
#define L3GD20_DPS_TO_RADS         (0.017453293F)  

//Variables globales
char *axes[] = { "X: ", "Y: ", "Z: " };

/* Convert degrees to radians */
#define d2r(d) ((d) * 6.2831853 / 360.0)

void spi_setup(void)
{
    rcc_periph_clock_enable(RCC_SPI5);
    rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOF);

	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOC, GPIO1);

    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7 | GPIO8 | GPIO9);   
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);

    spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
	spi_set_full_duplex_mode(SPI5);
	spi_set_unidirectional_mode(SPI5);
	spi_enable_software_slave_management(SPI5);
	spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
    SPI_I2SCFGR(SPI5) &= ~SPI_I2SCFGR_I2SMOD;
	spi_enable(SPI5);

	/* Enable GPIOG clock. */
	rcc_periph_clock_enable(RCC_GPIOG);

	/* Set GPIO13 (in GPIO port G) to 'output push-pull'. */
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO14);
}

static void my_usart_print_int(uint32_t usart, int32_t value)
{
	int8_t i;
	char buffer[25];
	int8_t nr_digits = 0;

	if (value < 0) {
		usart_send_blocking(usart, '-');
		value = value * -1;
	}

	if (value == 0) {
		usart_send_blocking(usart, '0');
	}

	while (value > 0) {
		buffer[nr_digits++] = "0123456789"[value % 10];
		value /= 10;
	}

	for (i = nr_digits-1; i >= 0; i--) {
		usart_send_blocking(usart, buffer[i]);
	}

	usart_send_blocking(usart, '\r');
	usart_send_blocking(usart, '\n');
}

int print_decimal(int);
int print_decimal(int num)
{
	char	is_signed = 0;
	int		len = 0;
	int		ndx = 0;
	char	buf[10];

	if (num < 0) 
    {
		is_signed++;
		num = 0 - num;
	}
	buf[ndx++] = '\000';
	do 
    {
		buf[ndx++] = (num % 10) + '0';
		num = num / 10;
	} 
    while (num != 0);
	ndx--;
	if (is_signed != 0) 
    {
		console_putc('-');
		len++;
	}
	while (buf[ndx] != '\000') {
		console_putc(buf[ndx--]);
		len++;
	}
	return len; 
}


/*
 * This is our example, the heavy lifing is actually in lcd-spi.c but
 * this drives that code.
 */
int main(void)
{
	int p1, p2, p3;
	clock_setup();
	console_setup(115200);

	spi_setup();
	//adc_setup();

    gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_CTRL_REG1); 
	spi_read(SPI5);
	spi_send(SPI5, GYR_CTRL_REG1_PD | GYR_CTRL_REG1_XEN |
			GYR_CTRL_REG1_YEN | GYR_CTRL_REG1_ZEN |
			(3 << GYR_CTRL_REG1_BW_SHIFT));
	spi_read(SPI5);
	gpio_set(GPIOC, GPIO1); 

    gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_CTRL_REG4);
	spi_read(SPI5);
	spi_send(SPI5, (1 << GYR_CTRL_REG4_FS_SHIFT));
	spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);

    console_puts("X\tY\tZ\n");

	sdram_init();
	lcd_spi_init();
	console_puts("LCD Initialized\n");
	console_puts("Should have a checker pattern, press any key to proceed\n");
	msleep(2000);

	int16_t X;
    int16_t Y;
    int16_t Z;

	X=0;
	Y=0;
	Z=0;

/*	(void) console_getc(1); */
	gfx_init(lcd_draw_pixel, 240, 320);
	gfx_fillScreen(LCD_GREY);
	gfx_fillRoundRect(10, 10, 220, 220, 5, LCD_WHITE);
	gfx_drawRoundRect(10, 10, 220, 220, 5, LCD_RED);
	gfx_fillCircle(20, 250, 10, LCD_RED);
	gfx_fillCircle(120, 250, 10, LCD_GREEN);
	gfx_fillCircle(220, 250, 10, LCD_BLUE);
	gfx_setTextSize(2);
	gfx_setCursor(15, 25);
	gfx_puts("Sismografo");
	gfx_setTextSize(1);
	gfx_setCursor(15, 49);
	gfx_puts("Laboratorio micros");
	gfx_setCursor(15, 65);
	gfx_puts("Laboratorio 4");
	gfx_setCursor(15, 79);
	gfx_puts("Jose Mario Gonzalez");
	gfx_setCursor(15, 89);
	gfx_puts("Jose Carlos Gonzalez");
	gfx_setCursor(15, 99);
	gfx_puts("28/05/2023");
	lcd_show_frame();
	console_puts("Now it has a bit of structured graphics.\n");
	console_puts("Press a key for some simple animation.\n");
	msleep(4000);
/*	(void) console_getc(1); */
	gfx_setTextColor(LCD_YELLOW, LCD_BLACK);
	gfx_setTextSize(3);
	while (1) {
		uint8_t t;
        uint8_t who;
		char buff_x[25];
		char buff_y[25];
		char buff_z[25];
		char lcd_out[3];

		sprintf(buff_x, "Eje X: %.d", X);
		gfx_fillScreen(LCD_GREY);

		gfx_setTextSize(2);
		gfx_setCursor(15, 25);
		gfx_puts(buff_x);

		sprintf(buff_y, "Eje Y: %.d", Y);
		gfx_setTextSize(2);
		gfx_setCursor(15, 55);
		gfx_puts(buff_y);

		sprintf(buff_z, "Eje Z: %.d", Z);
		gfx_setTextSize(2);
		gfx_setCursor(15, 85);
		gfx_puts(buff_z);

		lcd_show_frame();

		gpio_clear(GPIOC, GPIO1);             
		spi_send(SPI5, GYR_WHO_AM_I | 0x80);
		spi_read(SPI5); 
		spi_send(SPI5, 0);    
		who=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_STATUS_REG | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		t=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_TEMP | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		t=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);  

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_X_L | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		X=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_X_H | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		X|=spi_read(SPI5) << 8;
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_Y_L | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		Y=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_Y_H | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		Y|=spi_read(SPI5) << 8;
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_Z_L | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		Z=spi_read(SPI5);
		gpio_set(GPIOC, GPIO1);

		gpio_clear(GPIOC, GPIO1);
		spi_send(SPI5, GYR_OUT_Z_H | GYR_RNW);
		spi_read(SPI5);
		spi_send(SPI5, 0);
		Z|=spi_read(SPI5) << 8;
		gpio_set(GPIOC, GPIO1);

        X = X*L3GD20_SENSITIVITY_250DPS;
        Y = Y*L3GD20_SENSITIVITY_250DPS;
        Z = Z*L3GD20_SENSITIVITY_250DPS;

	    print_decimal(X);  console_puts("\t");
        print_decimal(Y);  console_puts("\t");
        print_decimal(Z);  console_puts("\n");

		int i;
		for (i = 0; i < 80000; i++)    /* Wait a bit. */
			__asm__("nop");

		msleep(100);


	}
}
