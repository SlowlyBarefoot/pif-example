// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exPmlcdI2c_H_
#define _exPmlcdI2c_H_
#include "Arduino.h"
//add your includes for the project exPmlcdI2c here

#include "pifComm.h"
#include "pifI2c.h"

//end of add your includes here


//add your function definitions for the project exPmlcdI2c here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
BOOL actPmlcdI2cWrite(PIF_stI2c *pstOwner, uint16_t usSize);

//Do not add code below this line
#endif /* _exPmlcdI2c_H_ */
