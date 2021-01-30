// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exLed1_H_
#define _exLed1_H_
#include "Arduino.h"
//add your includes for the project exLed1 here

#include "pif.h"

//end of add your includes here


//add your function definitions for the project exLed1 here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
void actLedRGBState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);

//Do not add code below this line
#endif /* _exLed1_H_ */
