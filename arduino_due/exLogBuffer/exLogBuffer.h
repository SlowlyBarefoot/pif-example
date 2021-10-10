// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exLogBuffer_H_
#define _exLogBuffer_H_
#include "Arduino.h"
//add your includes for the project exLogBuffer here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exLogBuffer here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedL(SWITCH sw);

//Do not add code below this line
#endif /* _exLogBuffer_H_ */
