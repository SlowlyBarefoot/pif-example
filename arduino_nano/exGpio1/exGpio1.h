// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exGpio1_H_
#define _exGpio1_H_
#include "Arduino.h"
//add your includes for the project exGpio1 here


//end of add your includes here

#include "pif.h"

//add your function definitions for the project exGpio1 here

void actLogPrint(char *pcString);
void actGpioLedL(PIF_usId usPifId, uint8_t ucState);
void actGpioLedRG(PIF_usId usPifId, uint8_t ucState);
uint8_t actGpioSwitch(PIF_usId usPifId);

//Do not add code below this line
#endif /* _exGpio1_H_ */
