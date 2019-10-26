/*
 * DAC.c
 *
 *  Created on: Oct 18, 2019
 *      Author: gonzalosilva
 */

#include "DAC.h"

#define DAC_DATL_DATA0_WIDTH 8

void DAC_init(void){
	SIM->SCGC2 |= SIM_SCGC2_DAC0_MASK;
	//SIM_SCGC2 |= SIM_SCGC2_DAC1_MASK;

	DAC0->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
	//DAC1->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
}

void DAC_set_data(DAC_t dac, DACData_t data){
	dac->DAT[0].DATL = (uint8_t)DAC_DATL_DATA0((uint8_t)data);
	dac->DAT[0].DATH = DAC_DATH_DATA1((data >> DAC_DATL_DATA0_WIDTH) & DAC_DATH_DATA1_MASK);
}

void DAC_write(DAC_t dac, float data){

}
