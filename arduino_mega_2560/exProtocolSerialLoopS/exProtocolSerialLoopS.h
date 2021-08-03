// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialLoopS_H_
#define _exProtocolSerialLoopS_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialLoopS here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exProtocolSerialLoopS here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
uint16_t actSerial1SendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial1ReceiveData(PIF_stComm *pstOwner, uint8_t *pucData);
uint16_t actSerial2SendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial2ReceiveData(PIF_stComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exProtocolSerialLoopS_H_ */