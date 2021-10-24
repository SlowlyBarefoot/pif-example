// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exXmodemSerialRx_H_
#define _exXmodemSerialRx_H_
#include "Arduino.h"
//add your includes for the project exXmodemSerialRx here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exXmodemSerialRx here

void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifId usPifId);
uint16_t actXmodemSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actXmodemReceiveData(PifComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exXmodemSerialRx_H_ */
