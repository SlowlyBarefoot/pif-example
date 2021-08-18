// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSensorThreshold2P_H_
#define _exSensorThreshold2P_H_
#include "Arduino.h"
//add your includes for the project exSensorThreshold2P here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exSensorThreshold2P here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t taskLedToggle(PIF_stTask *pstTask);
uint16_t taskSensorAcquisition(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exSensorThreshold2P_H_ */
