// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch2_H_
#define _exSwitch2_H_
#include "Arduino.h"
//add your includes for the project exSwitch2 here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exSwitch2 here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
void evtSwitchAcquire(void *pvIssuer);
void evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer);
void evtTiltSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer);

//Do not add code below this line
#endif /* _exSwitch2_H_ */
