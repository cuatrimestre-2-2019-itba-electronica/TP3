/***************************************************************************//**
  @file     3LedDisp.h
  @brief    Leds driver
  @author   Matias Pierdominici
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_



#include <stdbool.h>




#define SYSTICK_ISR_FREQUENCY_HZ 50000L
#define NUMBER_OF_CALLBACKS 10
typedef void (*SysTick_funcallback)(void);

/**
 * @brief Initialize SysTic driver
 * @return Initialization and registration succeed
 */
bool SysTick_Init (void);

/**
 * @brief append function to be called at e frecuency of SYSTICK_ISR_FREQUENCY_HZ
 * @return true if the callback was registred
 * @param callback
 */
bool SysTick_append(SysTick_funcallback func);

/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_
