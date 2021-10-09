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

#include "pifComm.h"

//add your function definitions for the project exGpio1 here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actGpioLedL(PifId usPifId, uint8_t ucState);
void actGpioLedRG(PifId usPifId, uint8_t ucState);
uint8_t actGpioSwitch(PifId usPifId);

//Do not add code below this line
#endif /* _exGpio1_H_ */
