// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exTaskDelay_H_
#define _exTaskDelay_H_
#include "Arduino.h"
//add your includes for the project exTaskDelay here

#include "pif.h"

//end of add your includes here


//add your function definitions for the project exTaskDelay here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
void actLedRGBState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);

//Do not add code below this line
#endif /* _exTaskDelay_H_ */
