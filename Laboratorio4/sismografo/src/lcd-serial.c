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

/****************************/
/*Laboratorio 4 - Sismografo 
  IE-0624 
  Authors: Jose Mario Gonzalez
  		   Jose Carlos Gonzalez
  Date: 04/06/2023
*/

//Libraries
#include <libopencm3/stm32/rcc.h> //Configuracion del reloj
#include <libopencm3/stm32/gpio.h> //Configuracion de GPIOS
#include <libopencm3/stm32/spi.h> //
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
#define GYR_RNW			(1 << 7) /* Write when zero */  
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
//char *axes[] = { "X: ", "Y: ", "Z: " };
float_t voltage;
uint8_t open_serial;
float_t bat_percent;


void spi_setup(void)
{
	//El giroscopio y la pantalla estan conectados al SPI5
	//Configuracion del reloj que alimenta los GPIOs
    rcc_periph_clock_enable(RCC_SPI5); //Reloj para SPI
    rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOF);

	//Setea el modo de operacion de los pines utilizados
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOC, GPIO1);

    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7 | GPIO8 | GPIO9);   
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);

	//Inicializacion del giroscopio
	//Inicializacion y configuracion del protocolo SPI
    spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
	spi_set_full_duplex_mode(SPI5);
	spi_set_unidirectional_mode(SPI5);
	spi_enable_software_slave_management(SPI5);
	spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
	spi_enable(SPI5);

	//Habilita reloj para GPIOG y lo habilita como salida
	//Esta configuracion se establece para la activacion de los LEDs de advertencia
	//Estos LEDs estan en la tarjeta en GPIO13 y GPIO14
	rcc_periph_clock_enable(RCC_GPIOG);
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO14);
}

//Funcion para habilitar el GPIO0 como entrada para el boton
//Tomado de ejemplo de la libreria de examples (button/button.c)
static void button_setup(void)
{
	//Setea el reloj del periferico A
	rcc_periph_clock_enable(RCC_GPIOA);

	//Habilita el modo de operacion del puerto como entrada
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
	
}


//Esta funcion toma un numero entero y lo convierte en una representacion decimal de cadena
//Imprime la cadena resultante en consola a traves de console_putc()
//Ejemplo tomado de la libreria de examples (spi/spi-mems.c)
//Se usa para la impresion en consola una vez habilitada la comunicacion
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
	return len; //Devuelve la representacion decimal, retornando el numero de caracteres impresos
}

// Procesamiento del voltaje de a bateria mediante el convertidor
// analogico digital
// Configura el reloj para el adc y luego configura el modo de operacion de sus pines
static void adc_setup(void)
{
	rcc_periph_clock_enable(RCC_ADC1); //Habilita el reloj para el periferico ADC1
  	rcc_periph_clock_enable(RCC_GPIOA); //Habilita el reloj para el puerto GPIOA
	//gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO1); //Configura pin GPIO1 como entrada analogica
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0); //Configura pin GPIO0 como entrada analogica

	adc_power_off(ADC1); //Apaga el ADC (durante configuracion)
	adc_disable_scan_mode(ADC1); //Deshabilita el modo escaneo
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC); //Configura el tiempo de muestreo en todos los canales (3 ciclos)

	adc_power_on(ADC1); //Enciende el ADC, despues de ser configurado

} 

//Lee un valor de la entrada analogica y lo retorna
//Convierte el valor recibido en un valor digital
static uint16_t read_adc_naiive(uint8_t channel)
{
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}

//Funcion de lectura y escalamiento del dato analogico de voltaje
//Recibe 5v y maneja esto en escala de 0 a 9
void battery_read(void){
	voltage = read_adc_naiive(0)*9/4095.0;
}


int main(void)
{
	//int p1, p2, p3;
	//Empieza la configuracion del reloj
	clock_setup();
	console_setup(115200);

	//Empieza la configuracion del adc y de los puertos spi
	adc_setup();
	spi_setup();

	//Empieza la configuracion del boton
	//button_setup();
	open_serial = 1; //Habilita la comunicacion

	//EMpieza la configuracion de lectura y escritura del giroscopio
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

    //console_puts("X\tY\tZ\n");

	//Iniccializa la sdram y la pantalla lcd
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

	//Diseno de la "portada del sistema"
	//Ejemplo tomado de lcd-serial.c
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
	gfx_puts("IE-0624");
	gfx_setCursor(15, 79);
	gfx_puts("Laboratorio 4");
	gfx_setCursor(15, 89);
	gfx_puts("Jose Mario Gonzalez");
	gfx_setCursor(15, 99);
	gfx_puts("Jose Carlos Gonzalez");
	gfx_setCursor(15, 109);
	gfx_puts("28/05/2023");
	lcd_show_frame();
	console_puts("Now it has a bit of structured graphics.\n");
	console_puts("Press a key for some simple animation.\n");
	msleep(4000);

	gfx_setTextColor(LCD_YELLOW, LCD_BLACK);
	gfx_setTextSize(3);
	while (1) {
		uint8_t t; //variable temp
        uint8_t who; //registro who am I
		char buff_x[25]; //Almacena valor de eje X
		char buff_y[25]; //Almacena valor de eje Y
		char buff_z[25]; //Almacena valor de eje Z
		char buff_batt[20]; //Almacena valor de la bateria
		char buff_percent[30]; 
	

		battery_read();

		//Lee el boton USR del pin GPIO0
		//Si se toca el boton cambia el valor de open serial
		//Habilita o inhabilita la comunicacion mediante el boton
		/*if(gpio_get(GPIOA, GPIO0)){
			if(open_serial){
				open_serial = 0;
			}
			else{
				open_serial = 1;
			}
		}*/

		bat_percent = voltage * 11.1;

		sprintf(buff_x, "Eje X: %.d", X); //Formatea una cadena de texto para mostrar valor de X
		gfx_fillScreen(LCD_BLACK); //Llena el fondo de la pantalla de color gris

		gfx_setTextSize(2); //Se ajusta tamano de text en 2
		gfx_setCursor(15, 25); //Establece coordenada en pantalla de la variable X
		gfx_puts(buff_x); //Imprime en pantalla variable X

		sprintf(buff_y, "Eje Y: %.d", Y); //Formatea una cadena de texto para mostrar valor de Y
		gfx_setTextSize(2); //Se ajusta tamano de text en 2
		gfx_setCursor(15, 55); //Establece coordenada en pantalla de la variable Y
		gfx_puts(buff_y); //Imprime en pantalla variable Y

		sprintf(buff_z, "Eje Z: %.d", Z); //Formatea una cadena de texto para mostrar valor de Z
		gfx_setTextSize(2); //Se ajusta tamano de text en 2
		gfx_setCursor(15, 85); //Establece coordenada en pantalla de la variable Z
		gfx_puts(buff_z); //Imprime en pantalla variable Z

		sprintf(buff_batt, "Voltage: %.2f", voltage); //Formatea una cadena de texto para mostrar valor de la bateria
		gfx_setTextSize(1); //Se ajusta tamano de text en 2
		gfx_setCursor(25, 150); //Establece coordenada en pantalla de la bateria
		gfx_puts(buff_batt);  //Imprime en pantalla variable buff_batt

		sprintf(buff_percent, "Batt percentage: %.2f", bat_percent); //Formatea una cadena de texto para mostrar valor de la bateria
		gfx_setTextSize(1); //Se ajusta tamano de text en 2
		gfx_setCursor(25, 185); //Establece coordenada en pantalla de la bateria
		gfx_puts(buff_percent);  //Imprime en pantalla variable buff_batt

		lcd_show_frame();

		//Tomado de ejemplo de libreria de examples (/f3/spi/spi.c)
		//Registro Who am I
		gpio_clear(GPIOC, GPIO1);            //Pone en bajo chip select
		spi_send(SPI5, GYR_WHO_AM_I | 0x80); //Se indica cual registro se quiere leer, establece el bit mas significativo a 1 con la mascara
		spi_read(SPI5); 				     //Se lee la respuesta
		spi_send(SPI5, 0);    				 //Se envia un 0
		who=spi_read(SPI5);					 //Se la respuesta (valor del registro)
		gpio_set(GPIOC, GPIO1);				 //Se pone en alto el chip select

		//REgistro de status
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_STATUS_REG | GYR_RNW);   //Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0); 							//Se envia un 0
		t=spi_read(SPI5);                           //Se lee valor del registro
		gpio_set(GPIOC, GPIO1);						//SE pone en alto el pin CS

		//Registro temp
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_TEMP | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		t=spi_read(SPI5);							//Se lee valor del registr
		gpio_set(GPIOC, GPIO1);  					//Se pone en alto el pin CS

		//Parte baja X
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_X_L | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		X=spi_read(SPI5);							//Se lee valor del registro
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Parte alta X
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_X_H | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		X|=spi_read(SPI5) << 8;						//Se lee valor del registro, se desplaza a la izquierda y se combina con el valor bajo
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Parte baja Y
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_Y_L | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		Y=spi_read(SPI5);							//Se lee valor del registro
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Parte alta Y
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_Y_H | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		Y|=spi_read(SPI5) << 8;						//Se lee valor del registro, se desplaza a la izquierda y se combina con el valor bajo
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Parte baja Z
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_Z_L | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		Z=spi_read(SPI5);							//Se lee valor del registro
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Parte alta Z
		gpio_clear(GPIOC, GPIO1);					//Pone en bajo chip select
		spi_send(SPI5, GYR_OUT_Z_H | GYR_RNW);		//Se indica cual registro se quiere leer
		spi_read(SPI5);								//Se lee respuesta
		spi_send(SPI5, 0);							//Se envia un 0
		Z|=spi_read(SPI5) << 8;						//Se lee valor del registro, se desplaza a la izquierda y se combina con el valor bajo
		gpio_set(GPIOC, GPIO1);						//Se pone en alto el pin CS

		//Multiplica los valores de los ejes por la sensibilidad del giroscopio
        X = X*L3GD20_SENSITIVITY_250DPS; 
        Y = Y*L3GD20_SENSITIVITY_250DPS;
        Z = Z*L3GD20_SENSITIVITY_250DPS;

		//Tomado de ejemplo de libreria de examples (f4/spi/spi-mems.c)
		//Imprime los valores en consola
		//cuando se habilita la conexion serial
		if(open_serial){
			print_decimal(X);   //Convierte X en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(Y);   //Convierte Y en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(Z);   //Convierte Z en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(voltage); //Convierte el valor de voltaje en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime nueva linea
			print_decimal(bat_percent); //Convierte el valor de voltaje en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\n"); //Imprime nueva linea
			gpio_toggle(GPIOG, GPIO13); //Enciende LED de comunicacion activa
		}
		else{
			//Si la comunicación serial no está habilitada
			print_decimal(0);   //Convierte X en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(0);   //Convierte Y en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(0);   //Convierte Z en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\t"); //Imprime espacio en la consola entre valores
			print_decimal(0); //Convierte el valor de voltaje en cadena de caracteres e imprime su representacion decimal en consola
			console_puts("\n"); //Imprime nueva linea
			gpio_clear(GPIOG, GPIO13); //Apaga LED de comunicacion activa
		}
		
		if(voltage < 7){
			gpio_toggle(GPIOG, GPIO14); //Enciende el LED de bateria baja
		}
		else{
			gpio_clear(GPIOG, GPIO14); //Apaga el LED de bateria baja
		}

		int i;
		for (i = 0; i < 80000; i++)    /* Wait a bit. */
			__asm__("nop");

		msleep(100);


	}
}
