// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exLogPrint_H_
#define _exLogPrint_H_
#include "Arduino.h"
//add your includes for the project exLogPrint here

#include "communication/pif_uart.h"

//end of add your includes here


//add your function definitions for the project exLogPrint here

uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedL(SWITCH sw);

//Do not add code below this line
#endif /* _exLogPrint_H_ */
