/*
 * PIT.c
 *
 *  Created on: Oct 26, 2019
 *      Author: gonzalosilva
 */

#define CLOCK_FRECUENCY 50000000



#include "PIT.h"

void PIT0_IRQHandler(void);
void PIT1_IRQHandler(void);
void PIT2_IRQHandler(void);
void PIT3_IRQHandler(void);

static void PIT_call_handler (uint8_t timer_num);

static uint8_t pit_IRQ[] = {PIT0_IRQn, PIT1_IRQn, PIT2_IRQn, PIT3_IRQn};


static PIT_funcallback funcs[4];

void PIT_init (void){
	SIM->SCGC6 |= SIM_SCGC6_PIT(1);
	PIT->MCR &= (~PIT_MCR_MDIS_MASK);
}


void PIT_IRQ (PIT_funcallback callback, uint8_t timer_num){
	reset_flag(timer_num);
	PIT->CHANNEL[timer_num].TCTRL |= PIT_TCTRL_TIE_MASK;
	funcs[timer_num] = callback;
	NVIC_EnableIRQ(pit_IRQ[timer_num]);

}

void PIT_set_timer (uint32_t frecuency, uint8_t timer_num){
	PIT->CHANNEL[timer_num].LDVAL = (uint32_t)((CLOCK_FRECUENCY/frecuency) - 1);
	//PIT->CHANNEL[timer_num].TCTRL = PIT_TCTRL_TIE_MASK;
	PIT->CHANNEL[timer_num].TCTRL |= PIT_TCTRL_TEN(1);
}


uint32_t check_flag(uint8_t timer_num){
	return PIT->CHANNEL[timer_num].TFLG & PIT_TFLG_TIF_MASK;
}

void reset_flag(uint8_t timer_num){
	PIT->CHANNEL[timer_num].TFLG &= PIT_TFLG_TIF_MASK;
}

void PIT_call_handler (uint8_t timer_num) {
	funcs[timer_num]();
	reset_flag(timer_num);
	/*for (int i = 0; i < 32; i++){
		if ((PORT_PCR_ISF_MASK & ports[port]->PCR[i])){
			(*(funcs[port][i]))();
			ports[port]->PCR[i] |= PORT_PCR_ISF_MASK;
		}
	}*/
}

void PIT0_IRQHandler(void){
	PIT_call_handler(0);
}
void PIT1_IRQHandler(void){
	PIT_call_handler(1);
}
void PIT2_IRQHandler(void){
	PIT_call_handler(2);
}
void PIT3_IRQHandler(void){
	PIT_call_handler(3);
}

