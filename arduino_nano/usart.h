#ifndef USART_H
#define USART_H


#include "pif_comm.h"


#define ASYNCHRONOUS					(0 << UMSEL00)		// USART Mode Selection

#define PARITY_DISABLED					(0 << UPM00)
#define PARITY_EVEN 					(2 << UPM00)
#define PARITY_ODD  					(3 << UPM00)
#define PARITY_DEFAULT 					PARITY_DISABLED		// USART Parity Bit Selection

#define STOP_BIT_ONE 					(0 << USBS0)
#define STOP_BIT_TWO 					(1 << USBS0)
#define STOP_BIT_DEFAULT				STOP_BIT_ONE      	// USART Stop Bit Selection

#define DATA_BIT_FIVE					(0 << UCSZ00)
#define DATA_BIT_SIX					(1 << UCSZ00)
#define DATA_BIT_SEVEN					(2 << UCSZ00)
#define DATA_BIT_EIGHT					(3 << UCSZ00)
#define DATA_BIT_DEFAULT				DATA_BIT_EIGHT 		// USART Data Bit Selection


#ifdef __cplusplus
extern "C" {
#endif

void USART_Init(uint32_t baud, uint8_t config, BOOL bRxInt);
BOOL USART_StartTransfer();
void USART_Send(PifComm *pstComm);
void USART_Receive(PifComm *pstComm);

#ifdef __cplusplus
}
#endif


#endif	// USART_H
