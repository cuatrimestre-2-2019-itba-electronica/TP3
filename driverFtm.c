/*
 * driverFtm.c
 *
 *  Created on: Oct 16, 2019
 *      Author: mlste
 */
#include "driverFtm.h"
#include "MK64F12.h"

#include "gpio.h"
#include "board.h"


#define DRIVER_FTM_CLK_SOURCE 1
#define DRIVER_FTM_CLK_TIMER_AMAUNT 4
#define DRIVER_FTM_CNTIN_INIT 0x0000
#define DRIVER_FTM_CNTIN_MOD 0xFFFF
#define DRIVER_FTM_PSC DRIVER_FTM_PSC_x32

typedef uint8_t pin_t;
static PORT_Type * ports[] = PORT_BASE_PTRS;
static uint32_t sim_port[] = {SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK};

static void driverFtmUpdateChanelCountInK(uint16_t k);
static void driverFtmSetChanelValue(uint16_t value,uint8_t chanel, uint8_t ftm);

typedef enum
{
	DRIVER_FTM_PSC_x1 = 0x00,
	DRIVER_FTM_PSC_x2 = 0x01,
	DRIVER_FTM_PSC_x4 = 0x02,
	DRIVER_FTM_PSC_x8 = 0x03,
	DRIVER_FTM_PSC_x16 = 0x04,
	DRIVER_FTM_PSC_x32 = 0x05,
	DRIVER_FTM_PSC_x64 = 0x06,
	DRIVER_FTM_PSC_x128 = 0x07,
} FTM_Prescal_t;


FTM_Type ** ftmTimers = FTM_BASE_PTRS;
uint32_t * clock_gates = {&(SIM->SCGC6), &(SIM->SCGC6), &(SIM->SCGC6),  &(SIM->SCGC3)};
uint32_t * clock_masks = {SIM_SCGC6_FTM0_MASK, SIM_SCGC6_FTM1_MASK, SIM_SCGC6_FTM2_MASK, SIM_SCGC3_FTM3_MASK};
uint8_t * irqEnable=FTM_IRQS;


static void setup_pin (pin_t pin){
	int pinPort = PIN2PORT(pin);
	int pinBit = PIN2NUM(pin);
	SIM->SCGC5 |= sim_port[pinPort]; //Habilito el clock al puerto correspondiente

	//configuro el pcr del pin
	ports[pinPort]->PCR[pinBit]= PORT_PCR_MUX(4) | \
			PORT_PCR_IRQC(0) | \
			PORT_PCR_PS(0)| \
			PORT_PCR_PE(0);


}


void driverFtmInit(int witchFtm){
	//clock gating
	gpioMode(PIN_SCK, OUTPUT);
	if(witchFtm>DRIVER_FTM_CLK_TIMER_AMAUNT){
		witchFtm=DRIVER_FTM_CLK_TIMER_AMAUNT;
	}
	SIM->SCGC6 |=SIM_SCGC6_FTM0_MASK;//clock gating del perfiferico
	//clock_gates[witchFtm] = (clock_gates[witchFtm] & ~clock_masks[witchFtm]) | clock_masks[witchFtm];
	/*
	ftmTimers[witchFtm]->MODE = (ftmTimers[witchFtm]->MODE & ~FTM_MODE_FTMEN_MASK) | FTM_MODE_FTMEN_MASK;//pongo el ftmen en 1
	ftmTimers[witchFtm]->SC = (ftmTimers[witchFtm]->SC & ~FTM_SC_CLKS_MASK) | FTM_SC_CLKS(DRIVER_FTM_CLK_SOURCE); //indico que utilizo el sitme clock 50M
	ftmTimers[witchFtm]->SC = (ftmTimers[witchFtm]->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(DRIVER_FTM_PSC_x2); //indico que utilizo el sitem clock 50M
	ftmTimers[witchFtm]->CNTIN = (ftmTimers[witchFtm]->CNTIN & ~FTM_CNTIN_INIT_MASK) | FTM_CNTIN_INIT(DRIVER_FTM_CNTIN_INIT);//seteo el valor inicial del contador
	ftmTimers[witchFtm]->MOD = (ftmTimers[witchFtm]->MOD & ~FTM_MOD_MOD_MASK)|FTM_MOD_MOD(DRIVER_FTM_CNTIN_MOD);//seteo el valor al que llega el contador

	ftmTimers[witchFtm]->SC = (ftmTimers[witchFtm]->SC & ~ FTM_SC_TOIE_MASK) | FTM_SC_TOIE_MASK;//habilito la interrupcion de overflow
	NVIC_EnableIRQ(irqEnable[witchFtm]);//habilito la interrupcion de ese periferico

	*/
	// Deshabilitar el periferico para la inicializacion
	FTM0->SC = 		(FTM0->SC 	 & ~FTM_SC_CLKS_MASK) 	| FTM_SC_CLKS(0);

	FTM0->MODE =	(FTM0->MODE  & ~FTM_MODE_FTMEN_MASK)| FTM_MODE_FTMEN_MASK;//pongo el ftmen en 1
	FTM0->SC = 		(FTM0->SC 	 & ~FTM_SC_PS_MASK) 	| FTM_SC_PS(DRIVER_FTM_PSC); //indico que utilizo el sitem clock 50M
	FTM0->SC = 		(FTM0->SC 	 & ~FTM_SC_TOIE_MASK) 	| FTM_SC_TOIE_MASK;//habilito la interrupcion de overflow
	FTM0->MOD = 	(FTM0->MOD 	 & ~FTM_MOD_MOD_MASK)	| FTM_MOD_MOD(DRIVER_FTM_CNTIN_MOD);//seteo el valor al que llega el contador
	FTM0->CNTIN = 	(FTM0->CNTIN & ~FTM_CNTIN_INIT_MASK)| FTM_CNTIN_INIT(DRIVER_FTM_CNTIN_INIT);//seteo el valor inicial del contador

	driverFtmSetCahnelOutputCompare(0);
	driverFtmSetChanelValue(100,0,0);
	//Elijo fuente de clock al final, porque al cambiarlo de 0 se prende
	FTM0->SC = 		(FTM0->SC 	 & ~FTM_SC_CLKS_MASK) 	| FTM_SC_CLKS(DRIVER_FTM_CLK_SOURCE); //indico que utilizo el sitme clock 50M
	setup_pin(PIN_FTM0C0);
	NVIC_EnableIRQ(FTM0_IRQn);//habilito la interrupcion de ese periferico

	//	FTM0->MODE = 	(FTM0->MODE	 & ~FTM_MODE_INIT_MASK)	| FTM_MODE_INIT_MASK;
}





void FTM0_DriverIRQHandler(void){
	uint8_t overFlowFlag=0;
	uint8_t chanel0CompFlag=0;

	overFlowFlag=FTM0->SC & FTM_SC_TOF_MASK;//veo si la interrupcion es de overflow

	chanel0CompFlag=FTM0->CONTROLS[0].CnSC & FTM_CnSC_CHF_MASK;

	if(overFlowFlag){
		FTM0->SC = (FTM0->SC & ~ FTM_SC_TOF_MASK) | FTM_SC_TOF(0);//bajo el flag de la irq

	}

	if(chanel0CompFlag){
		FTM0->CONTROLS[0].CnSC= (FTM0->CONTROLS[0].CnSC & ~FTM_CnSC_CHF_MASK)|FTM_CnSC_CHF(0); //reseteo el flag de la irq
		driverFtmUpdateChanelCountInK(100);
		gpioToggle(PIN_SCK);
	}

}

void driverFtmSetCahnelOutputCompare(uint8_t chanel){
	//steps
	//DECAPEN 0
	//COMBINE 0
	//CPWMS 0
	//MSNB:MSNA 01 *
	//ELSNB:ELSNA 01 *
	//tambien el chie

	FTM0->COMBINE=(FTM0->COMBINE & ~FTM_COMBINE_DECAPEN0_MASK)|FTM_COMBINE_DECAPEN0(0);
	FTM0->COMBINE=(FTM0->COMBINE & ~FTM_COMBINE_COMBINE0_MASK )|FTM_COMBINE_COMBINE0(0);

	FTM0->SC=(FTM0->SC & ~FTM_SC_CPWMS_MASK)|FTM_SC_CPWMS(0);

	FTM0->CONTROLS[chanel].CnSC = (FTM0->CONTROLS[chanel].CnSC & ~ FTM_CnSC_MSB_MASK)| FTM_CnSC_MSB(0);
	FTM0->CONTROLS[chanel].CnSC = (FTM0->CONTROLS[chanel].CnSC & ~ FTM_CnSC_MSA_MASK)| FTM_CnSC_MSA(1);

	FTM0->CONTROLS[chanel].CnSC = (FTM0->CONTROLS[chanel].CnSC & ~ FTM_CnSC_ELSB_MASK)| FTM_CnSC_ELSB(0);
	FTM0->CONTROLS[chanel].CnSC = (FTM0->CONTROLS[chanel].CnSC & ~ FTM_CnSC_ELSA_MASK)| FTM_CnSC_ELSA(1);

	FTM0->CONTROLS[chanel].CnSC = (FTM0->CONTROLS[chanel].CnSC & ~FTM_CnSC_CHIE_MASK )|FTM_CnSC_CHIE(1); //habilitio las interrupciones del canal



}


//value el valor que se setea en el contador
//chanel el canal deseado a setear el value
//ftm el ftma a usar todo:programarlo
static void driverFtmSetChanelValue(uint16_t value,uint8_t chanel, uint8_t ftm){
	FTM0->CONTROLS[chanel].CnV=(FTM0->CONTROLS[chanel].CnV & ~ FTM_CnV_VAL_MASK)|FTM_CnV_VAL(value);
}

static uint16_t driverFtmGetChanleValue(uint8_t chanel, uint8_t ftm){
	return FTM0->CONTROLS[chanel].CnV;
}

//devuelve el actual valor del free runing counter del ftm seleccionado todo: hacerlo para otros ftm
static uint16_t driverFtmGetCounterValue(uint8_t ftm){
	return FTM0->CNT;
}

static void driverFtmUpdateChanelCountInK(uint16_t k){
	driverFtmSetChanelValue(driverFtmGetChanleValue(0,0)+k,0,0);
}





