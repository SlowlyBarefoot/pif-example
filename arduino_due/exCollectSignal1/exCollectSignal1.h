// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exCollectSignal1_H_
#define _exCollectSignal1_H_
#include "Arduino.h"
//add your includes for the project exCollectSignal1 here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exCollectSignal1 here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint32_t unState);
void actLedRGBState(PIF_usId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
uint16_t taskTerminal(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exCollectSignal1_H_ */
