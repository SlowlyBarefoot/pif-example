// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exDutyMotorSpeedEnc_H_
#define _exDutyMotorSpeedEnc_H_
#include "Arduino.h"
//add your includes for the project exDutyMotorSpeedEnc here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exDutyMotorSpeedEnc here

void actLogPrint(char *pcString);
void taskTerminal(PIF_stTask *pstTask);
uint16_t actPhotoInterruptAcquire(PIF_usId usPifId);
void actSetDuty(uint16_t usDuty);
void actSetDirection(uint8_t ucDir);
void actOperateBreak(uint8_t ucState);
void actLedLState(PIF_usId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exDutyMotorSpeedEnc_H_ */
