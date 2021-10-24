// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exCollectSignal1_H_
#define _exCollectSignal1_H_
#include "Arduino.h"
//add your includes for the project exCollectSignal1 here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exCollectSignal1 here

void actLedLState(PifId usPifId, uint32_t unState);
void actGpioRGBState(PifId usPifId, uint8_t unState);
void actLedCollectState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifId usPifId);
uint16_t actPushSwitchCollectAcquire(PifId usPifId);
uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exCollectSignal1_H_ */
