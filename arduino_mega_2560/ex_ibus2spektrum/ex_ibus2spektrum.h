// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_ibus2spektrum_H_
#define _ex_ibus2spektrum_H_
#include "Arduino.h"
//add your includes for the project ex_ibus2spektrum here

#include "core/pif_comm.h"

//end of add your includes here


//add your function definitions for the project ex_ibus2spektrum here

uint16_t actLogSendData(PifComm* p_owner, uint8_t* p_buffer, uint16_t size);
void actLedLState(PifId id, uint32_t state);
BOOL actSerial1ReceiveData(PifComm* p_owner, uint8_t* p_data);
uint16_t actSerial2SendData(PifComm* p_owner, uint8_t* p_buffer, uint16_t size);

//Do not add code below this line
#endif /* _ex_ibus2spektrum_H_ */
