// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialM_H_
#define _exProtocolSerialM_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialM here

#include "communication/pif_uart.h"

//end of add your includes here


//add your function definitions for the project exProtocolSerialM here

void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actSerialSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerialReceiveData(PifUart *p_uart, uint8_t *pucData);

//Do not add code below this line
#endif /* _exProtocolSerialM_H_ */
