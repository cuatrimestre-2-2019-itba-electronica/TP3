#include <CMP.h>


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "CMP.h"
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static CMP_Type * 		const CMP_bases[] 	= CMP_BASE_PTRS;
static const uint8_t   	const irqEnable[]	= CMP_IRQS;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void CMP_init(CMP_initData_t * data)
{
	if (data == 0) {return;}

	uint8_t CMP_n = data->CMP_n;
	CMP_Type * CMP_base_ptr = CMP_bases[CMP_n];

	//clock gating
	SIM->SCGC4 |= SIM_SCGC4_CMP_MASK;

	//CR0//
	//hysteresis, filtrado desactivado
	CMP_base_ptr->CR0 = CMP_CR0_HYSTCTR(data->hysteresis) | CMP_CR0_FILTER_CNT(1);


	//FPR//

	//SCR//
	CMP_base_ptr->SCR = CMP_SCR_IEF_MASK | CMP_SCR_IER_MASK;

	//DACCR//
	//Configuracion del DAC
	//El nivel de comparacion se mapea del rango (0, 255) al rango (0, 64)
	CMP_base_ptr->DACCR = CMP_DACCR_VOSEL(data->level >> 2) | CMP_DACCR_VRSEL_MASK | CMP_DACCR_DACEN_MASK;

	//MUXCR//
	CMP_base_ptr->MUXCR = (CMP_base_ptr->MUXCR & ~CMP_MUXCR_PSEL_MASK) | CMP_MUXCR_PSEL(1);
	CMP_base_ptr->MUXCR = (CMP_base_ptr->MUXCR & ~CMP_MUXCR_MSEL_MASK) | CMP_MUXCR_MSEL(7);


	//habilitacion de la interrupcion
	NVIC_EnableIRQ(irqEnable[CMP_n]);

	//CR1//
	//enable del modulo
	//low power
	//enable del pin de salida
	CMP_base_ptr->CR1 = CMP_CR1_OPE(1) | CMP_CR1_EN(1);	//enable module
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void CMP0_IRQHandler(void){
	CMP_IRQHandler(0);
}

void CMP1_IRQHandler(void){
	CMP_IRQHandler(1);
}

void CMP2_IRQHandler(void){
	CMP_IRQHandler(2);
}
void CMP_IRQHandler(uint8_t CMP_n){
	//apago el flag
	CMP_bases[CMP_n]->SCR |= CMP_SCR_CFF_MASK | CMP_SCR_CFR_MASK;
}

/******************************************************************************/
