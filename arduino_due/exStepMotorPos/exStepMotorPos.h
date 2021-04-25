// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exStepMotorPos_H_
#define _exStepMotorPos_H_
#include "Arduino.h"
//add your includes for the project exStepMotorPos here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exStepMotorPos here

void actLogPrint(char *pcString);
void taskTerminal(PIF_stTask *pstTask);
uint16_t actPhotoInterruptAcquire(PIF_usId usPifId);
void actSetStep(uint16_t usPhase);
void actLedLState(PIF_usId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exStepMotorPos_H_ */
