/*
 * PIT.h
 *
 *  Created on: Oct 26, 2019
 *      Author: gonzalosilva
 */

#ifndef PIT_H_
#define PIT_H_

#include "MK64F12.h"

typedef PIT_Type *PIT_ptr;

typedef void (*PIT_funcallback)(void);

void PIT_init(void);
void PIT_set_timer (uint32_t frecuency, uint8_t timer_num);
void PIT_IRQ (PIT_funcallback callback, uint8_t timer_num);
uint32_t check_flag(uint8_t timer_num);
void reset_flag(uint8_t timer_num);

#endif /* PIT_H_ */
