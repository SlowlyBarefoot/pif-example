// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exServoMotor_H_
#define _exServoMotor_H_
#include "Arduino.h"
//add your includes for the project exServoMotor here

#include "pif.h"

//end of add your includes here


//add your function definitions for the project exServoMotor here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint32_t unState);
void actPulsePwm(SWITCH swValue);

//Do not add code below this line
#endif /* _exServoMotor_H_ */
