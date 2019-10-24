
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FTM.h"
#include <stdint.h>
#include "MK64F12.h"
//#include "board.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define FTM_MODULES_COUNT  4
#define FTM_CHANNELS_COUNT 8

#define FTM0_CH0_PIN PORTNUM2PIN(PC, 1) //CH0
#define FTM0_CH1_PIN PORTNUM2PIN(PC, 2) //CH1
#define FTM0_CH2_PIN PORTNUM2PIN(PC, 3) //CH2
#define FTM0_CH3_PIN PORTNUM2PIN(PC, 4) //CH3
#define FTM0_CH4_PIN PORTNUM2PIN(PA, 0) //CH5
#define FTM0_CH7_PIN PORTNUM2PIN(PA, 2) //CH7
//FTM3
#define FTM3_CH0_PIN PORTNUM2PIN(PD, 0) //CH0
#define FTM3_CH1_PIN PORTNUM2PIN(PD, 1) //CH1
#define FTM3_CH2_PIN PORTNUM2PIN(PD, 2) //CH2
#define FTM3_CH3_PIN PORTNUM2PIN(PD, 3) //CH3
#define FTM3_CH5_PIN PORTNUM2PIN(PC, 9) //CH5

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void FTM_setupChannel(uint8_t FTM_n, FTM_channelData_t * data);
static int  FTM_getMod(FTM_Type * FTM_base_ptr);
static int  FTM_getCntinInit(FTM_Type * FTM_base_ptr);
static int  FTM_getCnV(FTM_Type * FTM_base_ptr, uint8_t channel);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// Matriz con los callbacks de todos los canales.
// Nota: los canales 1 y 2 tienen menos canales que los canales 0 y 3, asi que
// va a haber posiciones de la matriz no usadas
static FTM_callback channelCallbacks[FTM_MODULES_COUNT][FTM_CHANNELS_COUNT];

// Matriz con los callbacks de todos los modulos. Se llama cuando hay overflow.
static FTM_callback moduleCallbacks[FTM_MODULES_COUNT];


static uint32_t combine_syncen_masks[] = {	FTM_COMBINE_SYNCEN0_MASK,
											FTM_COMBINE_SYNCEN1_MASK,
											FTM_COMBINE_SYNCEN2_MASK,
											FTM_COMBINE_SYNCEN3_MASK};

FTM_Type * 		const FTM_bases[] 	= FTM_BASE_PTRS;
__IO uint32_t * const clock_gates[] = {&(SIM->SCGC6), &(SIM->SCGC6), &(SIM->SCGC6),  &(SIM->SCGC3)};
const uint32_t  const clock_masks[] = {SIM_SCGC6_FTM0_MASK, SIM_SCGC6_FTM1_MASK, SIM_SCGC6_FTM2_MASK, SIM_SCGC3_FTM3_MASK};
const uint8_t   const irqEnable[]	= FTM_IRQS;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void FTM_init(FTM_initData_t * data)
{


	if(data == 0) { return; }

	portsSetupPin(FTM0_CH0_PIN, 4);
	//todo: hacer para mas de un canal y mas de un ftm, y poner un define para el mux

	uint8_t FTM_n = data->FTM_n;
	FTM_Type * FTM_base_ptr = FTM_bases[FTM_n];

	//clock gating
	*(clock_gates[FTM_n]) = (*(clock_gates[FTM_n])  & ~clock_masks[FTM_n]) 			| clock_masks[FTM_n];

	//guardo el callback
	moduleCallbacks[FTM_n] = data->overflowCallback;

	// Deshabilitar el periferico para la inicializacion desconectandolo de los clocks
	FTM_base_ptr->SC = 		(FTM_base_ptr->SC 	 	& ~FTM_SC_CLKS_MASK) 	 		| FTM_SC_CLKS(0);

	//pongo el ftmen en 1
	FTM_base_ptr->MODE =	(FTM_base_ptr->MODE  	& ~FTM_MODE_FTMEN_MASK)  		| FTM_MODE_FTMEN_MASK;
	//prescaler
	FTM_base_ptr->SC = 		(FTM_base_ptr->SC 	 	& ~FTM_SC_PS_MASK) 		 		| FTM_SC_PS(data->prescaler);
	//interupciones habilitadas/deshabilitadas
	FTM_base_ptr->SC = 		(FTM_base_ptr->SC 	 	& ~FTM_SC_TOIE_MASK) 	 		| FTM_SC_TOIE(data->interruptsEnabled);
	//mod: valor del overflow
	FTM_base_ptr->MOD = 	(FTM_base_ptr->MOD 	 	& ~FTM_MOD_MOD_MASK)	 		| FTM_MOD_MOD(data->mod);
	//cuenta inicial del contador
	FTM_base_ptr->CNTIN = 	(FTM_base_ptr->CNTIN 	& ~FTM_CNTIN_INIT_MASK)	 		| FTM_CNTIN_INIT(data->cntin);

	//restart registers con PWMLOAD activado o desactivado
	FTM_base_ptr->PWMLOAD = (FTM_base_ptr->PWMLOAD 	& ~FTM_PWMLOAD_LDOK_MASK)		| FTM_PWMLOAD_LDOK(data->restartRegisters);



	for (int i = 0; i < data->channelConfigsBuffLen; ++i) {
		FTM_setupChannel(FTM_n, &(data->channelConfigsBuff[i]));
	}


	//habilito la interrupcion de ese periferico
	NVIC_EnableIRQ(irqEnable[FTM_n]);

	//Elijo fuente de clock al final, porque al cambiarlo de 0 se prende
	FTM_base_ptr->SC = 		(FTM_base_ptr->SC 	 & ~FTM_SC_CLKS_MASK) 	| FTM_SC_CLKS(data->clockSource);//clock source

}

void FTM_setDuty(uint8_t FTM_n, uint8_t channel, uint8_t duty)
{
	FTM_Type * FTM_base_ptrs[] = FTM_BASE_PTRS;
	int mod = FTM_getMod(FTM_base_ptrs[FTM_n]);
	int cntinInit = FTM_getCntinInit(FTM_base_ptrs[FTM_n]);
	int Cnv_value  = cntinInit + (int)((mod-cntinInit)*duty/256);

	//cambio de duty con trigger por software
	FTM_base_ptrs[FTM_n]->PWMLOAD = (FTM_base_ptrs[FTM_n]->PWMLOAD 	& ~FTM_PWMLOAD_LDOK_MASK) 	| ~FTM_PWMLOAD_LDOK(1);
	FTM_base_ptrs[FTM_n]->CONTROLS[channel].CnV	= (FTM_base_ptrs[FTM_n]->CONTROLS[channel].CnV  & ~FTM_CnV_VAL_MASK) | FTM_CnV_VAL(Cnv_value);
	FTM_base_ptrs[FTM_n]->SYNC	  = (FTM_base_ptrs[FTM_n]->SYNC		& ~FTM_SYNC_SWSYNC_MASK)	| FTM_SYNC_SWSYNC(1);

}

void FTM_shutdownChannel(uint8_t FTM_n, uint8_t channel)
{

}

void FTM_shutdown(uint8_t FTM_n)
{

}

void FTM0_DriverIRQHandler(void)
{
	FTM_DriverIRQHandler(0);
}

void FTM1_DriverIRQHandler(void)
{
	FTM_DriverIRQHandler(1);
}

void FTM2_DriverIRQHandler(void)
{
	FTM_DriverIRQHandler(2);
}

void FTM3_DriverIRQHandler(void){
	FTM_DriverIRQHandler(3);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


static void FTM_setupChannel(uint8_t FTM_n, FTM_channelData_t * data)
{

	FTM_Type * const FTM_bases[] = FTM_BASE_PTRS;
	FTM_Type * FTM_base_ptr = FTM_bases[FTM_n];
	uint8_t channel = data->channel;
	FTM_config_t config = data->config;

	uint32_t combine_decapen_mask;
	uint32_t combine_combine_mask;

	// interrupciones habilitadas/deshabilitadas
	FTM_base_ptr->CONTROLS[channel].CnSC= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_CHIE_MASK)	| FTM_CnSC_CHIE(data->interruptsEnabled);


	// seteo callback de interrupcion:
	channelCallbacks[FTM_n][channel] = data->callback;
	switch(FTM_n)
	{
		case 0:
			combine_combine_mask = FTM_COMBINE_COMBINE0_MASK;
			combine_decapen_mask = FTM_COMBINE_DECAPEN0_MASK;
			break;
		case 1:
			combine_combine_mask = FTM_COMBINE_COMBINE1_MASK;
			combine_decapen_mask = FTM_COMBINE_DECAPEN1_MASK;
			break;
		case 2:
			combine_combine_mask = FTM_COMBINE_COMBINE2_MASK;
			combine_decapen_mask = FTM_COMBINE_DECAPEN2_MASK;
			break;
		case 3:
			combine_combine_mask = FTM_COMBINE_COMBINE3_MASK;
			combine_decapen_mask = FTM_COMBINE_DECAPEN3_MASK;
			break;
		default:
			return;
	}

	FTM_base_ptr->COMBINE = (FTM_base_ptr->COMBINE	& ~combine_syncen_masks[(int)(channel/2)]) | combine_syncen_masks[(int)(channel/2)];

	switch(data->mode)
	{
		case FTM_MODE_OUTPUT_COMPARE:
			////Configuro ouput compare:
			// * Decapen = 0
			// * Combine = 0
			// * CPWMS = 0
			// * MSnB = 0
			// * MSnA = 1
			// * CnV = data->outputcompareData.value
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_decapen_mask);
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_combine_mask);
			FTM_base_ptr->SC		= (FTM_base_ptr->SC 	 & ~FTM_SC_CPWMS_MASK);
			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSA_MASK)| FTM_CnSC_MSA(1);
			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSB_MASK)| FTM_CnSC_MSB(0);
			FTM_base_ptr->CONTROLS[channel].CnV		= (FTM_base_ptr->CONTROLS[channel].CnV  & ~FTM_CnV_VAL_MASK) | FTM_CnV_VAL(data->outputCompareData.value);

			if(config == FTM_CONFIG_TOGGLE_OUTPUT){
				////Configuro Toggle output to match:
				// * ELSnB = 1
				// * ELSnA = 0
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(0);
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(1);
			} else if (config == FTM_CONFIG_SET_OUTPUT) {
			} else if (config == FTM_CONFIG_CLEAR_OUTPUT) {
			}
			break;
		case FTM_MODE_INPUT_CAPTURE:
			break;
		case FTM_MODE_CPWM:
			break;
		case FTM_MODE_EPWM:
			////Configuro edge aligned pwm:
			// * Decapen = 0
			// * Combine = 0
			// * CPWMS = 0
			// * MSnB = 1
			// * MSnA = X
			// * CnV = duty (calculado en funcion de mod,cntin, y duty)
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_decapen_mask);
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_combine_mask);
			FTM_base_ptr->SC		= (FTM_base_ptr->SC 	 & ~FTM_SC_CPWMS_MASK);
//			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSA_MASK)| FTM_CnSC_MSA(1);
			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSB_MASK)| FTM_CnSC_MSB(1);

			//Habilito/deshabilito las funciones de PWMLOAD para este canal:
			int pwmLoadChannelSelectMask = 1 << channel;
			int pwmLoadChannelSelect 	 = data->EPWM_data.restartRegisters << channel;
			FTM_base_ptr->PWMLOAD = (FTM_base_ptr->PWMLOAD & ~pwmLoadChannelSelectMask )	| pwmLoadChannelSelect;

			//sincronizacion de registros con trigger por software para PWM
			FTM_base_ptr->MODE =	(FTM_base_ptr->MODE  	& ~FTM_MODE_PWMSYNC_MASK)  		| FTM_MODE_PWMSYNC(1);
			FTM_base_ptr->SYNCONF = (FTM_base_ptr->SYNCONF	& ~FTM_SYNCONF_SWWRBUF_MASK)	| FTM_SYNCONF_SWWRBUF_MASK;
			FTM_base_ptr->SYNCONF = (FTM_base_ptr->SYNCONF	& ~FTM_SYNCONF_SWRSTCNT_MASK)	| FTM_SYNCONF_SWRSTCNT_MASK;
			FTM_base_ptr->SYNCONF = (FTM_base_ptr->SYNCONF	& ~FTM_SYNCONF_SYNCMODE_MASK)	| FTM_SYNCONF_SYNCMODE(data->EPWM_data.restartRegisters);
			FTM_base_ptr->SYNCONF = (FTM_base_ptr->SYNCONF 	& ~FTM_SYNCONF_CNTINC_MASK) 	| FTM_SYNCONF_CNTINC_MASK;


			//Duty cycle:
			FTM_setDuty(FTM_n, channel, data->EPWM_data.duty);

			if(config == FTM_CONFIG_HIGH_TRUE){
				////Configuro High-true:
				// * ELSnB = 1
				// * ELSnA = 0
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(0);
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(1);
			} else if (config == FTM_CONFIG_LOW_TRUE) {
				////Configuro High-true:
				// * ELSnB = X
				// * ELSnA = 1
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(1);
//				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(1);
			}
			break;
		default:
			return;
	}

}

static int FTM_getMod(FTM_Type * FTM_base_ptr)
{
	return (FTM_base_ptr->MOD & FTM_MOD_MOD_MASK) >> FTM_MOD_MOD_SHIFT;
}

static int FTM_getCntinInit(FTM_Type * FTM_base_ptr)
{
	return (FTM_base_ptr->CNTIN & FTM_CNTIN_INIT_MASK) >> FTM_CNTIN_INIT_SHIFT;
}

static int FTM_getCnV(FTM_Type * FTM_base_ptr, uint8_t channel)
{
	return FTM_base_ptr->CONTROLS[channel].CnV;
}

void FTM_DriverIRQHandler(uint8_t FTM_n)
{
	//Para todas las interrupciones apago el flag y llamo al callback correspondiente.

	//si hubo interrupcion por overflow:
	if(FTM_bases[FTM_n]->SC & FTM_SC_TOF_MASK){
		FTM_bases[FTM_n]->SC = (FTM_bases[FTM_n]->SC & ~ FTM_SC_TOF_MASK) | FTM_SC_TOF(0);
		if(moduleCallbacks[FTM_n]){
			moduleCallbacks[FTM_n]();
		}
	}

	int channel_count = 2;
	if(FTM_n == 0 || FTM_n == 3){
		channel_count = 8;
	}

	//si hubo interrupcion en un canal:
	for(int i = 0; i < channel_count; i++){
		if(FTM_bases[FTM_n]->CONTROLS[i].CnSC & FTM_CnSC_CHF_MASK){
			FTM_bases[FTM_n]->CONTROLS[i].CnSC = (FTM_bases[FTM_n]->CONTROLS[i].CnSC & ~FTM_CnSC_CHF_MASK) | FTM_CnSC_CHF(0);
			if(channelCallbacks[FTM_n][i]){
				channelCallbacks[FTM_n][i]();
			}
		}
	}
}

/******************************************************************************/
