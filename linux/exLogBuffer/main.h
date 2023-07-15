#ifndef MAIN_H_
#define MAIN_H_


#include "communication/pif_uart.h"


uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);


#endif /* MAIN_H_ */
