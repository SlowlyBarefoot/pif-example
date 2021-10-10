// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exDutyMotorPos_H_
#define _exDutyMotorPos_H_
#include "Arduino.h"
//add your includes for the project exDutyMotorPos here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exDutyMotorPos here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);
uint16_t actPhotoInterruptAcquire(PifId usPifId);
void actSetDuty(uint16_t usDuty);
void actSetDirection(uint8_t ucDir);
void actOperateBreak(uint8_t ucState);
void actLedLState(PifId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exDutyMotorPos_H_ */
