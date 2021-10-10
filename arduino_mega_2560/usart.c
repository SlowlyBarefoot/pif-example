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
} stUsart;

static stUsart s_stUsart[4] = {
		{ &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0 },
		{ &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1 },
		{ &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2 },
		{ &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3 }
};


void USART_Init(int port, uint32_t baud, uint8_t config, BOOL bRxInt)
{
	stUsart *p_stUsart = &s_stUsart[port];
	uint16_t usPrescaler = (F_CPU / 4 / (baud) - 1) / 2;

	// Set Baud Rate
	*p_stUsart->ubrrh = usPrescaler >> 8;
	*p_stUsart->ubrrl = usPrescaler & 0xFF;

	*p_stUsart->ucsra = 1 << U2X0;

	// Set Frame Format
	*p_stUsart->ucsrc = ASYNCHRONOUS | config;

	// Enable Receiver and Transmitter
	*p_stUsart->ucsrb = (1 << RXEN0) | (1 << TXEN0);

	// Enable Rx complete interrupt
	if (bRxInt)	*p_stUsart->ucsrb |= (1 << RXCIE0);
}

BOOL USART_StartTransfer(int port)
{
	*s_stUsart[port].ucsrb |= (1 << UDRIE0); // Enables the Interrupt
	return TRUE;
}

void USART_Send(int port, PifComm *pstComm)
{
	stUsart *p_stUsart = &s_stUsart[port];
	uint8_t ucData, ucState;

	ucState = pifComm_SendData(pstComm, &ucData);
	if (ucState & PIF_COMM_SEND_DATA_STATE_DATA) {
		*p_stUsart->udr = ucData;

#ifdef MPCM0
		*p_stUsart->ucsra = (*p_stUsart->ucsra & ((1 << U2X0) | (1 << MPCM0))) | (1 << TXC0);
#else
		*p_stUsart->ucsra = (*p_stUsart->ucsra & ((1 << U2X0) | (1 << TXC0)));
#endif
	}
	if (ucState & PIF_COMM_SEND_DATA_STATE_EMPTY) {
		pifComm_FinishTransfer(pstComm);
		*p_stUsart->ucsrb &= ~(1 << UDRIE0); // Disables the Interrupt, uncomment for one time transmission of data
	}
}

void USART_Receive(int port, PifComm *pstComm)
{
	stUsart *p_stUsart = &s_stUsart[port];

	if (!(*p_stUsart->ucsra & (1 << UPE0))) {
		pifComm_ReceiveData(pstComm, *p_stUsart->udr);
	}
	else {
		*p_stUsart->udr;
	}
}