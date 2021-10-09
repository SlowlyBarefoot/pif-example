// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exAds1115_H_
#define _exAds1115_H_
#include "Arduino.h"
//add your includes for the project exAds1115 here

#include "pifComm.h"
#include "pifI2c.h"

//end of add your includes here


//add your function definitions for the project exAds1115 here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
BOOL actAds1115Read(PIF_stI2c *pstOwner, uint16_t usSize);
BOOL actAds1115Write(PIF_stI2c *pstOwner, uint16_t usSize);

//Do not add code below this line
#endif /* _exAds1115_H_ */
