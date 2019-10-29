/***************************************************************************//**
  @file     UART.c
  @brief    UART Driver for K64F. Non-Blocking
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "hardware.h"
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_CANT_IDS		5 // Numero of UARTs

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum{ no_parity, even_parity, odd_parity} uart_parity;

typedef struct {
	uart_parity parity;
    unsigned int baudrate;
    unsigned int stopBit;
} uart_cfg_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, etc.)
*/
void uartInit (uint8_t id, uart_cfg_t config);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
*/
uint8_t uartWriteMsg(uint8_t id, const char* msg, uint8_t cant);


#endif // _UART_H_
