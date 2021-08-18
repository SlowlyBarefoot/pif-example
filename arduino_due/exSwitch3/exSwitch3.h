// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch3_H_
#define _exSwitch3_H_
#include "Arduino.h"
//add your includes for the project exSwitch3 here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exSwitch3 here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedState(PIF_usId usPifId, uint32_t unState);
void evtSwitchAcquire(void *pvIssuer);

//Do not add code below this line
#endif /* _exSwitch3_H_ */
