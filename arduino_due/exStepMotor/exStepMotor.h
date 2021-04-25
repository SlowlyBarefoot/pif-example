// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exStepMotor_H_
#define _exStepMotor_H_
#include "Arduino.h"
//add your includes for the project exStepMotor here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exStepMotor here

void actLogPrint(char *pcString);
void taskTerminal(PIF_stTask *pstTask);
void actSetStep(uint16_t usPhase);
void actLedLState(PIF_usId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exStepMotor_H_ */
