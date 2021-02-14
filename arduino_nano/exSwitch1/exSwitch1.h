// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch1_H_
#define _exSwitch1_H_
#include "Arduino.h"
//add your includes for the project exSwitch1 here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exSwitch1 here

void actLogPrint(char *pcString);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
void evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer);
uint16_t actTiltSwitchAcquire(PIF_usId usPifId);
void evtTiltSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer);
void taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exSwitch1_H_ */
