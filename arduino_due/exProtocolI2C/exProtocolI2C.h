// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolI2C_H_
#define _exProtocolI2C_H_
#include "Arduino.h"
//add your includes for the project exProtocolI2C here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exProtocolI2C here

void actLogPrint(char *pcString);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState);
void taskI2C(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exProtocolI2C_H_ */
