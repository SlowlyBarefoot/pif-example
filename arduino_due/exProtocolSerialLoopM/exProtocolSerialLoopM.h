// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialLoopM_H_
#define _exProtocolSerialLoopM_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialLoopM here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exProtocolSerialLoopM here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
SWITCH actPushSwitchAcquire(PIF_usId usPifId);
void taskSerial1(PIF_stTask *pstTask);
void taskSerial2(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exProtocolSerialLoopM_H_ */
