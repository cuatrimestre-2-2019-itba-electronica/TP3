#include "uart.h"

#define UART_HAL_DEFAULT_BAUDRATE		9600

enum { PA, PB, PC, PD, PE };

#define PORTNUM2PIN(p,n)    (((p)<<5) + (n))
#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F)

/* UART0 PINS */
#define UART0_TX			PORTNUM2PIN(PB, 17)
#define UART0_RX			PORTNUM2PIN(PB, 16)
#define UART0_ALT			3

/* UART1 PINS */
#define UART1_TX			PORTNUM2PIN(PC, 4)
#define UART1_RX			PORTNUM2PIN(PC, 3)
#define UART1_ALT			3

/* UART2 PINS */
#define UART2_TX			PORTNUM2PIN(PD, 3)
#define UART2_RX			PORTNUM2PIN(PD, 2)
#define UART2_ALT			3

/* UART3 PINS */
#define UART3_TX			PORTNUM2PIN(PC, 17)
#define UART3_RX			PORTNUM2PIN(PC, 16)
#define UART3_ALT			3

/* UART4 PINS */
#define UART4_TX			PORTNUM2PIN(PE, 24)
#define UART4_RX			PORTNUM2PIN(PE, 25)
#define UART4_ALT			3

static PORT_Type * ports[] = PORT_BASE_PTRS;

// TX variables for circular buffer
static char buffer_TX[1024];
static int ptrR_TX = 0;
static int ptrW_TX = 0;
static int N_TX = 0;

// RX variables for circular buffer
static char buffer_RX[1024];
static int ptrR_RX = 0;
static int ptrW_RX = 0;
static int N_RX = 0;

void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate);
uint8_t check_UART_ID(uint8_t id);

UART_Type * UART_list[5] = {UART0, UART1, UART2, UART3, UART4};
enum IRQn UART_IRQn_list[5] = {UART0_RX_TX_IRQn, UART1_RX_TX_IRQn, UART2_RX_TX_IRQn, UART3_RX_TX_IRQn, UART4_RX_TX_IRQn};

void uartInit (uint8_t id, uart_cfg_t config){

	id = check_UART_ID(id);
	UART_Type * uartch = UART_list[id];
	int portNum;
	int pinNum;

	switch (id){
	case 0:
		portNum = PIN2PORT(UART0_TX);
		pinNum = PIN2NUM(UART0_TX);
		ports[portNum]->PCR[pinNum] = 0x00;

		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART0_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		portNum = PIN2PORT(UART0_RX);
		pinNum = PIN2NUM(UART0_RX);
		ports[portNum]->PCR[pinNum] = 0x00;
		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART0_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
		break;
	case 1:
		portNum = PIN2PORT(UART1_TX);
		pinNum = PIN2NUM(UART1_TX);
		ports[portNum]->PCR[pinNum] = 0x00;

		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART1_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		portNum = PIN2PORT(UART1_RX);
		pinNum = PIN2NUM(UART1_RX);
		ports[portNum]->PCR[pinNum] = 0x00;
		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART1_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
		break;
	case 2:
		portNum = PIN2PORT(UART2_TX);
		pinNum = PIN2NUM(UART2_TX);
		ports[portNum]->PCR[pinNum] = 0x00;

		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART2_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		portNum = PIN2PORT(UART2_RX);
		pinNum = PIN2NUM(UART2_RX);
		ports[portNum]->PCR[pinNum] = 0x00;
		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART2_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
		break;
	case 3:
		portNum = PIN2PORT(UART3_TX);
		pinNum = PIN2NUM(UART3_TX);
		ports[portNum]->PCR[pinNum] = 0x00;

		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART3_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		portNum = PIN2PORT(UART3_RX);
		pinNum = PIN2NUM(UART3_RX);
		ports[portNum]->PCR[pinNum] = 0x00;
		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART3_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
		break;
	case 4:
		portNum = PIN2PORT(UART4_TX);
		pinNum = PIN2NUM(UART4_TX);
		ports[portNum]->PCR[pinNum] = 0x00;

		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART4_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		portNum = PIN2PORT(UART4_RX);
		pinNum = PIN2NUM(UART4_RX);
		ports[portNum]->PCR[pinNum] = 0x00;
		ports[portNum]->PCR[pinNum] |= PORT_PCR_MUX(UART4_ALT);
		ports[portNum]->PCR[pinNum] |= PORT_PCR_IRQC(0);

		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
		break;
	}

	uartch->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

	/* Configure the UART for 8-bit mode, no parity */
	 /* We need all default settings, so entire register is cleared */

	UART_SetBaudRate(uartch, config.baudrate);
	uartch->C1 = 0;

	uartch->C2 = (UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK);
	NVIC_EnableIRQ(UART_IRQn_list[id]);
}

void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate){
	uint16_t sbr, brfa;
	uint32_t clock;

	clock = ((uart == UART0) || (uart == UART1)) ? (__CORE_CLOCK__):(__CORE_CLOCK__>>1);

	baudrate = ((baudrate == 0)?(UART_HAL_DEFAULT_BAUDRATE):
			((baudrate > 0x1FFF)?(UART_HAL_DEFAULT_BAUDRATE):(baudrate)));

	sbr = clock / (baudrate << 4);
	brfa = (clock << 1) / baudrate - (sbr << 5);

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}

uint8_t uartWriteMsg(uint8_t id, const char* msg, uint8_t cant){
	id = check_UART_ID(id);
	NVIC_DisableIRQ(UART_IRQn_list[id]);

	for(uint8_t i = 0; i<cant;i++){
		buffer_TX[ptrW_TX] = msg[i];
		if(ptrW_TX++ == 1024) ptrW_TX = 0;
		N_TX++;
	}

	UART_Type * uartch = UART_list[id];

	uartch->C2 |= UART_C2_TIE_MASK;

	NVIC_EnableIRQ(UART_IRQn_list[id]);
	return 1;
}

uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant){
	id = check_UART_ID(id);
	NVIC_DisableIRQ(UART_IRQn_list[id]);

	uint8_t n = 0;

	while (cant--) {
		if (N_RX == 0) {
			break;
		}
		*msg++ = buffer_RX[ptrR_RX];
		if (ptrR_RX++ == 1024) {
			ptrR_RX = 0;
		}
		N_RX--;
		n++;
	}

	NVIC_EnableIRQ(UART_IRQn_list[id]);
	return n;
}

__ISR__ UART0_RX_TX_IRQHandler (void)
{
	unsigned char tmp;

	tmp=UART0->S1;

	// Si es RX

	if(tmp & UART_S1_RDRF_MASK){
		buffer_RX[ptrW_RX] = UART0->D;
		ptrW_RX++;
		N_RX++;
	}

	// Si es TX
	if (tmp & UART_S1_TDRE_MASK) {
		if (N_TX <= 1){
			UART0->C2 &= ~UART_C2_TIE_MASK;
		}
		if (N_TX) {
			N_TX--;
			UART0->D = (uint8_t) buffer_TX[ptrR_TX]; // Transmit
			ptrR_TX++;
		}
	}
}

uint8_t check_UART_ID(uint8_t id){
	return (id>UART_CANT_IDS || id<0) ? 0 : id;
}
