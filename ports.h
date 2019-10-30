#ifndef _PORTS_H_
#define _PORTS_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// Ports
enum { PA, PB, PC, PD, PE };

// Convert port and number into pin ID
// Ex: PTB5  -> PORTNUM2PIN(PB,5)  -> 0x25
//     PTC22 -> PORTNUM2PIN(PC,22) -> 0x56
#define PORTNUM2PIN(p,n)    (((p)<<5) + (n))
#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F)


//FTM0:
#define FTM0_PIN_MUX 4
#define FTM0_CH0_PIN PORTNUM2PIN(PC, 1) //CH0
#define FTM0_CH1_PIN PORTNUM2PIN(PC, 2) //CH1
#define FTM0_CH2_PIN PORTNUM2PIN(PC, 3) //CH2
#define FTM0_CH3_PIN PORTNUM2PIN(PC, 4) //CH3
#define FTM0_CH4_PIN 0xFF //CH4
#define FTM0_CH5_PIN PORTNUM2PIN(PA, 0) //CH5
#define FTM0_CH6_PIN 0xFF //CH6
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

typedef uint8_t pin_t;
typedef void (*pinIrqFun_t)(void);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

static uint32_t FTM0_pins[] = {	FTM0_CH0_PIN,
								FTM0_CH1_PIN,
								FTM0_CH2_PIN,
								FTM0_CH3_PIN,
								FTM0_CH4_PIN,
								FTM0_CH5_PIN,
								FTM0_CH6_PIN,
								FTM0_CH7_PIN
							};


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void portsSetupPin(pin_t pin, uint8_t mux);


/*******************************************************************************
 ******************************************************************************/

#endif // _PORTS_H_
