// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exXmodemSerialRx_H_
#define _exXmodemSerialRx_H_
#include "Arduino.h"
//add your includes for the project exXmodemSerialRx here

#include "pifRingBuffer.h"
#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exXmodemSerialRx here

void actLogPrint(char *pcString);
BOOL actXmodemSendData(PIF_stRingBuffer *pstBuffer);
void actXmodemReceiveData(PIF_stRingBuffer *pstBuffer);
void taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exXmodemSerialRx_H_ */
