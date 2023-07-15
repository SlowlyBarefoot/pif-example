#ifndef MAIN_H_
#define MAIN_H_


#include "communication/pif_uart.h"


uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t actSerialSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerialReceiveData(PifUart *p_uart, uint8_t *pucData);


#endif /* MAIN_H_ */
