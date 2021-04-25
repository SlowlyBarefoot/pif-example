// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialS_H_
#define _exProtocolSerialS_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialS here

#include "pifRingBuffer.h"

//end of add your includes here


//add your function definitions for the project exProtocolSerialS here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint32_t unState);
BOOL actSerialSendData(PIF_stRingBuffer *pstBuffer);
void actSerialReceiveData(PIF_stRingBuffer *pstBuffer);

//Do not add code below this line
#endif /* _exProtocolSerialS_H_ */
