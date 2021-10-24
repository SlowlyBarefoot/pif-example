// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch1_H_
#define _exSwitch1_H_
#include "Arduino.h"
//add your includes for the project exSwitch1 here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exSwitch1 here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t actPushSwitchAcquire(PifId usPifId);
void evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer);
uint16_t actTiltSwitchAcquire(PifId usPifId);
void evtTiltSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer);
uint16_t taskLedToggle(PifTask *pstTask);

//Do not add code below this line
#endif /* _exSwitch1_H_ */
