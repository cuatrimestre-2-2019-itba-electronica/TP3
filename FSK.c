/*
 * FSK.c
 *
 *  Created on: Oct 25, 2019
 *      Author: gonzalosilva
 */
#include "FSK.h"
#include "DAC.h"
#include <math.h>
#include "SysTick.h"


static void FSK_generator(void);

static DAC_t dac;
static DACData_t data[AMOUNT_POINTS];
static float data_counter;
static uint8_t buffer[5]; //buffer circular en el que se guardan los datos a pasar a fsk
static int circ_counter; //contador para popear al buffer circular
static int buff_counter; //contador que recorre el buffer para convertir a fsk
static int size_buff; //tamano del buffer circular
static int var; 	//variable que se usa para hacer dos pasadas para hacer los dos periodos de la senoidal con mayor frecuencia

void FSK_init(void){
	DAC_init();
	dac = DAC0;
	SysTick_Init();
	SysTick_append(FSK_generator);
	for (int i = 0; i < AMOUNT_POINTS ; i++)
		data[i] = (uint16_t) (( 1+ sin((2 * 3.14 * i )/AMOUNT_POINTS)) * 0x7FF);
	size_buff = sizeof(buffer);
	for (int i = 0; i < size_buff ; i++)
		buffer[i] = 255;
}

void FSK_generator(void){
	if (!buffer[buff_counter]){
		if ((int)data_counter >= AMOUNT_POINTS-1){
			data_counter = 0;
			if (buff_counter == size_buff-1 && var == 1){
				buffer[buff_counter] = 255;
				buff_counter = 0;
				var=0;
			}
			else if (var ==1){
				buffer[buff_counter] = 255;
				buff_counter++;
				var = 0;
			}
			else
				var++;
		}
		else
			data_counter += 1.9;
	}
	else if(buffer[buff_counter] == 1){

		if ((int)data_counter >= AMOUNT_POINTS-1){
			data_counter = 0;
			if (buff_counter == size_buff-1){
				buffer[buff_counter] = 255;
				buff_counter = 0;
			}
			else{
				buffer[buff_counter] = 255;
				buff_counter++;
			}
		}
		else
			data_counter++;
	}
	DAC_set_data(dac, data[(int)data_counter]);

}

bool pop_to_buffer (bool num2pop){
	if(num2pop == 0 || num2pop == 1){
		if(buffer[circ_counter] != 255){
			return false;
		}
		else{
			buffer[circ_counter] = num2pop;
			if (circ_counter == (size_buff - 1))
				circ_counter = 0;
			else
				circ_counter++;

			return true;
		}
	}
	else
		return false;
}
