/*
 * DAC.h
 *
 *  Created on: Oct 18, 2019
 *      Author: gonzalosilva
 */

#ifndef DAC_H_
#define DAC_H_

#include "MK64F12.h"


typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;


void DAC_init(void);
void DAC_set_data(DAC_t dac, DACData_t data);
void DAC_write(DAC_t dac, float data);
#endif /* DAC_H_ */
