// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_buzzer_H_
#define _ex_buzzer_H_
#include "Arduino.h"
//add your includes for the project ex_buzzer here

#include "communication/pif_uart.h"

//end of add your includes here

//#define USE_SERIAL
#define USE_USART

//add your function definitions for the project ex_buzzer here

#ifdef USE_SERIAL
uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
#endif
#ifdef USE_USART
BOOL actLogStartTransfer(PifUart* p_uart);
#endif
void actBuzzerAction(PifId id, BOOL action);

//Do not add code below this line
#endif /* _ex_buzzer_H_ */
