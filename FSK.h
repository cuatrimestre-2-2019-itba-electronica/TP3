/*
 * FSK.h
 *
 *  Created on: Oct 25, 2019
 *      Author: gonzalosilva
 */

#ifndef FSK_H_
#define FSK_H_

#include <stdbool.h>


#define AMOUNT_POINTS 50
#define SAMPLE_FREQUENCY_1 (AMOUNT_POINTS * 1200)
#define SAMPLE_FREQUENCY_0 (int)(1.833333 * SAMPLE_FREQUENCY_1)


void FSK_init(void);


bool pop_to_buffer (bool num2pop); //si devuelve un 1 es que se pudo popear

#endif /* FSK_H_ */
