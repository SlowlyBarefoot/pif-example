// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_hc_sr04_H_
#define _ex_hc_sr04_H_
#include "Arduino.h"
//add your includes for the project ex_hc_sr04 here

#include "communication/pif_uart.h"

//end of add your includes here


//add your function definitions for the project ex_hc_sr04 here

uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size);
void actHcSr04Trigger(SWITCH state);

uint16_t taskLedToggle(PifTask* p_task);

//Do not add code below this line
#endif /* _ex_hc_sr04_H_ */
