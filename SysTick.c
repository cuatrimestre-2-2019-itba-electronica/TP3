#include "SysTick.h"
#include "hardware.h"
#include "board.h"
#include <stdlib.h>

static SysTick_funcallback callbacks[NUMBER_OF_CALLBACKS];
static uint8_t lastCallBackPos=0;

bool SysTick_Init (void)
{
	SysTick->CTRL = 0x00;
	SysTick->LOAD = 120000000/SYSTICK_ISR_FREQUENCY_HZ - 1;
	SysTick->VAL = 0x00;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk |
					SysTick_CTRL_ENABLE_Msk;
	for(int i=0;i<NUMBER_OF_CALLBACKS;i++){
		callbacks[i]=NULL;
	}

	return true;
}

__ISR__ SysTick_Handler (void)
{

	for(int i=0;i<(lastCallBackPos);i++){
			if(callbacks[i]!=NULL){
				((*callbacks[i])());
			}
		}


}

bool SysTick_append(SysTick_funcallback func){

	if(lastCallBackPos==NUMBER_OF_CALLBACKS){
		return false;
	}
	callbacks[lastCallBackPos]=func;
	lastCallBackPos++;

	return true;
}
