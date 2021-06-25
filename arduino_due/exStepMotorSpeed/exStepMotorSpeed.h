// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exStepMotorSpeed_H_
#define _exStepMotorSpeed_H_
#include "Arduino.h"
//add your includes for the project exStepMotorSpeed here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exStepMotorSpeed here

void actLogPrint(char *pcString);
uint16_t taskTerminal(PIF_stTask *pstTask);
uint16_t actPhotoInterruptAcquire(PIF_usId usPifId);
void actSetStep(uint16_t usPhase);
void actLedLState(PIF_usId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exStepMotorSpeed_H_ */
