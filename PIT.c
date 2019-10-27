/*
 * PIT.c
 *
 *  Created on: Oct 26, 2019
 *      Author: gonzalosilva
 */

#define CLOCK_FRECUENCY 50000000



#include "PIT.h"


void PIT_init (void){
	SIM->SCGC6 |= SIM_SCGC6_PIT(1);
	PIT->MCR &= (~PIT_MCR_MDIS_MASK);
}


void PIT_set_timer (uint8_t frecuency, uint8_t timer_num){

	PIT->CHANNEL[timer_num].LDVAL = (uint32_t)((CLOCK_FRECUENCY/frecuency) - 1);
	PIT->CHANNEL[timer_num].TCTRL = PIT_TCTRL_TIE_MASK;
	PIT->CHANNEL[timer_num].TCTRL |= PIT_TCTRL_TEN(1);
}


uint32_t check_flag(uint8_t timer_num){
	return PIT->CHANNEL[timer_num].TFLG & PIT_TFLG_TIF_MASK;
}

void reset_flag(uint8_t timer_num){
	PIT->CHANNEL[timer_num].TFLG &= PIT_TFLG_TIF_MASK;
}
