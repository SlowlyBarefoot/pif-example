#include <avr/io.h>      // Contains all the I/O Register Macros
#include <avr/interrupt.h> // Contains all interrupt vectors

#include "usart.h"


typedef struct {
	volatile uint8_t *ubrrh;
	volatile uint8_t *ubrrl;
	volatile uint8_t *ucsra;
	volatile uint8_t *ucsrb;
	volatile uint8_t *ucsrc;
	volatile uint8_t *udr;
	BOOL mpcm;					// multi processor communication mode
} stUsart;

static stUsart s_stUsart = {
		&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, FALSE
};


void USART_Init(uint32_t baud, uint8_t config, BOOL bRxInt)
{
	uint16_t usPrescaler = (F_CPU / 4 / (baud) - 1) / 2;

	// Set Baud Rate
	*s_stUsart.ubrrh = usPrescaler >> 8;
	*s_stUsart.ubrrl = usPrescaler & 0xFF;

	*s_stUsart.ucsra = 1 << U2X0;

	// Set Frame Format
	*s_stUsart.ucsrc = ASYNCHRONOUS | config;

	// Enable Receiver and Transmitter
	*s_stUsart.ucsrb = (1 << RXEN0) | (1 << TXEN0);

	// Enable Rx complete interrupt
	if (bRxInt)	*s_stUsart.ucsrb |= (1 << RXCIE0);
}

void USART_SetMultiProcessCommMode(BOOL mpcm)
{
	s_stUsart.mpcm = mpcm;
}

BOOL USART_StartTransfer()
{
	*s_stUsart.ucsrb |= (1 << UDRIE0); // Enables the Interrupt
	return TRUE;
}

void USART_Send(PifComm *pstComm)
{
	uint8_t ucData = 0, ucState;

	ucState = pifComm_GetTxByte(pstComm, &ucData);
	if (ucState & PIF_COMM_SEND_DATA_STATE_DATA) {
		*s_stUsart.udr = ucData;

#ifdef MPCM0
		*s_stUsart.ucsra = (*s_stUsart.ucsra & ((1 << U2X0) | (s_stUsart.mpcm << MPCM0))) | (1 << TXC0);
#else
		*s_stUsart.ucsra = (*s_stUsart.ucsra & ((1 << U2X0) | (1 << TXC0)));
#endif
	}
	if (ucState & PIF_COMM_SEND_DATA_STATE_EMPTY) {
		*s_stUsart.ucsrb &= ~(1 << UDRIE0); // Disables the Interrupt, uncomment for one time transmission of data
	}
}

void USART_Receive(PifComm *pstComm)
{
	if (!(*s_stUsart.ucsra & (1 << UPE0))) {
		pifComm_PutRxByte(pstComm, *s_stUsart.udr);
	}
	else {
		*s_stUsart.udr;
	}
}
