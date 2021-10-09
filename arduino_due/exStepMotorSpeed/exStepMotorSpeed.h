// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exStepMotorSpeed_H_
#define _exStepMotorSpeed_H_
#include "Arduino.h"
//add your includes for the project exStepMotorSpeed here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exStepMotorSpeed here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PIF_stComm *pstOwner, uint8_t *pucData);
uint16_t actPhotoInterruptAcquire(PifId usPifId);
void actSetStep(uint16_t usPhase);
void actLedLState(PifId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exStepMotorSpeed_H_ */
