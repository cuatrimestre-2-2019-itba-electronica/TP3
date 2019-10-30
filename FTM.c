
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





/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void 		FTM_setupChannel(uint8_t FTM_n, FTM_channelData_t * data);
static int  		FTM_getMod(FTM_Type * FTM_base_ptr);
static int  		FTM_getCntinInit(FTM_Type * FTM_base_ptr);
static int32_t 		FTM_getCnV(uint8_t FTM_n, uint8_t channel);
static void 		FTM_DriverIRQHandler(uint8_t FTM_n);
static FTM_mode_t 	FTM_getChannelMode(uint8_t FTM_n, uint8_t channel);
static void 		FTM_DriverIRQHandlerModule(uint8_t FTM_n);
static void 		FTM_DriverIRQHandlerChannel(uint8_t FTM_n);


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

// Se incrementa cada vez que hay un overflow del contador
static uint16_t overflowCounter[FTM_MODULES_COUNT];

// Guarda la cuenta de overflows del contador que habia la ultima vez que el
// input capture se triggereo (para cada canal)
static uint16_t overflowCounterWhenLastTriggered[FTM_MODULES_COUNT][FTM_CHANNELS_COUNT];

// Guarda el CnV que habia la ultima vez que el input capture se triggereo
// (para cada canal)
static uint16_t CnVWhenLastTriggered[FTM_MODULES_COUNT][FTM_CHANNELS_COUNT];

static int32_t overflowCounterDelta[FTM_MODULES_COUNT][FTM_CHANNELS_COUNT];

static int32_t CnVDelta[FTM_MODULES_COUNT][FTM_CHANNELS_COUNT];

static uint32_t combine_syncen_masks[] = {	FTM_COMBINE_SYNCEN0_MASK,
											FTM_COMBINE_SYNCEN1_MASK,
											FTM_COMBINE_SYNCEN2_MASK,
											FTM_COMBINE_SYNCEN3_MASK};


static FTM_Type * 		const FTM_bases[] 	= FTM_BASE_PTRS;
static __IO uint32_t * 	const clock_gates[] = {&(SIM->SCGC6), &(SIM->SCGC6), &(SIM->SCGC6),  &(SIM->SCGC3)};
static const uint32_t  	const clock_masks[] = {SIM_SCGC6_FTM0_MASK, SIM_SCGC6_FTM1_MASK, SIM_SCGC6_FTM2_MASK, SIM_SCGC3_FTM3_MASK};
static const uint8_t   	const irqEnable[]	= FTM_IRQS;



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void FTM_init(FTM_initData_t * data)
{


	if(data == 0) { return; }


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

void FTM_setDuty8bits(uint8_t FTM_n, uint8_t channel, uint8_t duty)
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

void FTM_setDuty12bits(uint8_t FTM_n, uint8_t channel, uint16_t duty)
{
	FTM_setDuty8bits(FTM_n, channel, duty>>4);
}



void FTM_shutdownChannel(uint8_t FTM_n, uint8_t channel)
{
	//todo:
}

void FTM_shutdown(uint8_t FTM_n)
{
	//todo:
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

void FTM3_DriverIRQHandler(void)
{
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
			////Configuro input capture:
			// * Decapen = 0
			// * Combine = 0
			// * CPWMS = 0
			// * MSnB = 0
			// * MSnA = 0
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_decapen_mask);
			FTM_base_ptr->COMBINE 	= (FTM_base_ptr->COMBINE & ~combine_combine_mask);
			FTM_base_ptr->SC		= (FTM_base_ptr->SC 	 & ~FTM_SC_CPWMS_MASK);
			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSA_MASK)| FTM_CnSC_MSA(0);
			FTM_base_ptr->CONTROLS[channel].CnSC	= (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_MSB_MASK)| FTM_CnSC_MSB(0);

			if(config == FTM_CONFIG_CAPTURE_RISING){
				////Configuro Capture on Rising Edge Only:
				// * ELSnB = 0
				// * ELSnA = 1
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(1);
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(0);
			} else if (config == FTM_CONFIG_CAPTURE_FALLING) {
				////Configuro Capture on Falling Edge Only:
				// * ELSnB = 1
				// * ELSnA = 0
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(0);
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(1);
			} else if (config == FTM_CONFIG_CAPTURE_BOTH) {
				////Configuro Capture on Rising or Falling Edge:
				// * ELSnB = 1
				// * ELSnA = 1
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSA_MASK)	| FTM_CnSC_ELSA(1);
				FTM_base_ptr->CONTROLS[channel].CnSC = (FTM_base_ptr->CONTROLS[channel].CnSC	& ~FTM_CnSC_ELSB_MASK)	| FTM_CnSC_ELSB(1);
			}
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
			FTM_setDuty8bits(FTM_n, channel, data->EPWM_data.duty);

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

static int32_t FTM_getCnV(uint8_t FTM_n, uint8_t channel)
{
	return FTM_bases[FTM_n]->CONTROLS[channel].CnV;
}

int32_t FTM_getPeriod(uint8_t FTM_n, uint8_t channel)
{
	return CnVDelta[FTM_n][channel] + FTM_bases[FTM_n]->MOD * overflowCounterDelta[FTM_n][channel] << 1;
}

static void FTM_DriverIRQHandler(uint8_t FTM_n)
{
	FTM_DriverIRQHandlerChannel(FTM_n);
	FTM_DriverIRQHandlerModule(FTM_n);
}

static void FTM_DriverIRQHandlerModule(uint8_t FTM_n)
{
	//si hubo interrupcion por overflow:
	if(FTM_bases[FTM_n]->SC & FTM_SC_TOF_MASK){
		// apago flag de interrupcion
		FTM_bases[FTM_n]->SC = (FTM_bases[FTM_n]->SC & ~ FTM_SC_TOF_MASK) | FTM_SC_TOF(0);
		if(moduleCallbacks[FTM_n]){
			//llamo al callback correspondiente si existe
			moduleCallbacks[FTM_n]();
			//incremento el contador de overflows
			overflowCounter[FTM_n]++;
		}
	}
}

static void FTM_DriverIRQHandlerChannel(uint8_t FTM_n)
{
	int channel_count = 2;
	if(FTM_n == 0 || FTM_n == 3){
		channel_count = 8;
	}

	//si hubo interrupcion en un canal:
	for(int i = 0; i < channel_count; i++){
		if(FTM_bases[FTM_n]->CONTROLS[i].CnSC & FTM_CnSC_CHF_MASK){
			// apago flag de interrupcion
			FTM_bases[FTM_n]->CONTROLS[i].CnSC = (FTM_bases[FTM_n]->CONTROLS[i].CnSC & ~FTM_CnSC_CHF_MASK) | FTM_CnSC_CHF(0);

			if(FTM_getChannelMode(FTM_n, i) == FTM_MODE_INPUT_CAPTURE){
				//Guardo datos de CnV
				int32_t CnV = FTM_getCnV(FTM_n, i);
				CnVDelta[FTM_n][i] = CnV - (int32_t)CnVWhenLastTriggered[FTM_n][i];
				CnVWhenLastTriggered[FTM_n][i] = CnV;

				//Guardo datos de overflow
				if(overflowCounter[FTM_n] > overflowCounterWhenLastTriggered[FTM_n][i]){
					overflowCounterDelta[FTM_n][i] = overflowCounter[FTM_n]  - overflowCounterWhenLastTriggered[FTM_n][i];
				} else {
					overflowCounterDelta[FTM_n][i] = overflowCounterWhenLastTriggered[FTM_n][i] - overflowCounter[FTM_n];
				}
				overflowCounterWhenLastTriggered[FTM_n][i] = overflowCounter[FTM_n];
			}

			if(channelCallbacks[FTM_n][i]){
				channelCallbacks[FTM_n][i]();
			}
		}
	}
}

static FTM_mode_t FTM_getChannelMode(uint8_t FTM_n, uint8_t channel)
{
	uint32_t CnSC = FTM_bases[FTM_n]->CONTROLS[channel].CnSC;
	if(FTM_bases[FTM_n]->SC & FTM_SC_CPWMS_MASK){
		return FTM_MODE_CPWM;
	} else if(CnSC & FTM_CnSC_MSB_MASK){
		return FTM_MODE_EPWM;
	} else if (CnSC & FTM_CnSC_MSA_MASK){
		return FTM_MODE_OUTPUT_COMPARE;
	} else if( (CnSC & FTM_CnSC_ELSB_MASK) || (CnSC & FTM_CnSC_ELSA_MASK) ){
		return FTM_MODE_INPUT_CAPTURE;
	} else {
		return FTM_MODE_N;
	}
}

/******************************************************************************/
