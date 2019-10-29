/*
 * adc.h
 *
 *  Created on: Oct 24, 2019
 *      Author: Matias Pierdominici
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include<stdbool.h>
typedef enum{ADC_0,ADC_1} adc;
typedef void (*adc_funcallback)(void);

void adcInitadcInit(adc whichAdc, uint8_t channel,adc_funcallback cb, bool usePit, uint8_t wPit);
void adcStartConvertion(adc whichAdc,uint8_t chanel);
bool adcEndConvertion(adc whichAdc);
uint16_t adcGetData(adc whichAdc);


#endif /* ADC_H_ */
