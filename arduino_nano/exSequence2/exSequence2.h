// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSequence2_H_
#define _exSequence2_H_
#include "Arduino.h"
//add your includes for the project exSequence2 here

#include "pif.h"

//end of add your includes here


//add your function definitions for the project exSequence2 here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
void actLedRGBState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
SWITCH actPushSwitchAcquire(PIF_usId usPifId);

//Do not add code below this line
#endif /* _exSequence2_H_ */
