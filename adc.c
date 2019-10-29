/*
 * adc.c
 *
 *  Created on: Oct 24, 2019
 *      Author: Matias Pierdominici
 */

#include "adc.h"
#include "hardware.h"
#include<stddef.h>
#define ADC_CLOCK_DIVIDER 0 //divide por 1
#define ADC_RESOLUTION 3//16bit de resolucion
	ADC_Type * adcs[2]=ADC_BASE_PTRS;
	uint32_t * perClock[2]={&(SIM->SCGC6),&(SIM->SCGC3)};
	uint32_t clockMask[2]={SIM_SCGC6_ADC0_MASK,SIM_SCGC3_ADC1_MASK};
	uint32_t enableIrq[2]={ADC0_IRQn,ADC1_IRQn};

adc_funcallback  adcIrq[]={NULL,NULL};
bool hayDatos[]={false,false};
uint16_t datosLeidos[]={0,0};
uint8_t pitSel[]={4,5,6,7};
void adcInit(adc whichAdc, uint8_t channel,adc_funcallback cb, bool usePit, uint8_t wPit){

	(*perClock[whichAdc])|=clockMask[whichAdc];//prendo el clock del periferico
	NVIC_EnableIRQ(enableIrq[whichAdc]);//habilito las irq del periferico
	if(cb !=NULL){
		adcIrq[whichAdc]=cb; //seteo el calback correpondiente
	}

	adcs[whichAdc]->SC1[0]=(adcs[whichAdc]->SC1[0] & ~ ADC_SC1_AIEN_MASK) | ADC_SC1_AIEN(1);//activo las irq
	adcs[whichAdc]->CFG1=(adcs[whichAdc]->CFG1 & ~ ADC_CFG1_ADIV_MASK)| ADC_CFG1_ADIV(ADC_CLOCK_DIVIDER);//divisor del clock
	adcs[whichAdc]->CFG1 = (adcs[whichAdc]->CFG1 & ~ADC_CFG1_MODE_MASK) | ADC_CFG1_MODE(ADC_RESOLUTION);//cantidad de bit de la conversion
	adcs[whichAdc]->CFG2 = (adcs[whichAdc]->CFG2 & ~ADC_CFG2_MUXSEL_MASK) | ADC_CFG2_MUXSEL(0);//ADXXa

	if(usePit){
		adcs[whichAdc]->SC2 = (adcs[whichAdc]->SC2 & ~ ADC_SC2_ADTRG_MASK ) | ADC_SC2_ADTRG(1);//selecciono hardaware trigger
		adcs[whichAdc]->SC1[0] = (adcs[whichAdc]->SC1[0] & ~ ADC_SC1_ADCH_MASK) | ADC_SC1_ADCH(channel);//inidico que canal se convierte
		if(whichAdc==0){
			SIM->SOPT7 = (SIM->SOPT7 & ~SIM_SOPT7_ADC0ALTTRGEN_MASK)| SIM_SOPT7_ADC0ALTTRGEN(1);
			SIM->SOPT7 = (SIM->SOPT7 & ~SIM_SOPT7_ADC0TRGSEL_MASK)| SIM_SOPT7_ADC0TRGSEL(pitSel[wPit]);//selecciono pit uno como trigger
		}else{
			SIM->SOPT7 = (SIM->SOPT7 & ~SIM_SOPT7_ADC1ALTTRGEN_MASK)| SIM_SOPT7_ADC1ALTTRGEN(1);
			SIM->SOPT7 = (SIM->SOPT7 & ~SIM_SOPT7_ADC1TRGSEL_MASK)| SIM_SOPT7_ADC1TRGSEL(pitSel[wPit]);//selecciono pit uno como trigger
		}

	}

}

void adcStartConvertion(adc whichAdc,uint8_t chanel){
	adcs[whichAdc]->SC1[0] = (adcs[whichAdc]->SC1[0] & ~ ADC_SC1_ADCH_MASK) | ADC_SC1_ADCH(chanel);
}


bool adcEndConvertion(adc whichAdc){
	//return adcs[whichAdc]->SC1[0] & ADC_SC1_COCO_MASK;
	return hayDatos[whichAdc];
}

uint16_t adcGetData(adc whichAdc){
	hayDatos[whichAdc]=false;
	return datosLeidos[whichAdc];
}

void ADC1_IRQHandler(void){
	//leo el dato
	hayDatos[1]=true;
	datosLeidos[1]=adcs[ADC_1]->R[0];
	if(adcIrq[1]!=NULL){
		(adcIrq[1])();
	}
}

void ADC0_IRQHandler(void){

	//leo el dato
	hayDatos[0]=true;
	datosLeidos[0]=adcs[ADC_0]->R[0];
	if(adcIrq[0]!=NULL){
		(adcIrq[0])();
	}
}

