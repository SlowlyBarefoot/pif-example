// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSensorThreshold1P_H_
#define _exSensorThreshold1P_H_
#include "Arduino.h"
//add your includes for the project exSensorThreshold1P here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exSensorThreshold1P here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t taskLedToggle(PifTask *pstTask);
uint16_t taskSensorAcquisition(PifTask *pstTask);

//Do not add code below this line
#endif /* _exSensorThreshold1P_H_ */
