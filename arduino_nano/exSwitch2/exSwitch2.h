// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch2_H_
#define _exSwitch2_H_
#include "Arduino.h"
//add your includes for the project exSwitch2 here

#include "pif.h"

//end of add your includes here


//add your function definitions for the project exSwitch2 here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
void evtSwitchAcquire(void *pvIssuer);
void evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer);
void evtTiltSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer);

//Do not add code below this line
#endif /* _exSwitch2_H_ */
