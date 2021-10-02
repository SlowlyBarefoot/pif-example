// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exCollectSignal2_H_
#define _exCollectSignal2_H_
#include "Arduino.h"
//add your includes for the project exCollectSignal2 here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exCollectSignal2 here

void actLedLState(PIF_usId usPifId, uint32_t unState);
void actGpioRGBState(PIF_usId usPifId, uint8_t unState);
void actLedCollectState(PIF_usId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
uint16_t actPushSwitchCollectAcquire(PIF_usId usPifId);
uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PIF_stComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exCollectSignal2_H_ */
