// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exXmodemSerialTx_H_
#define _exXmodemSerialTx_H_
#include "Arduino.h"
//add your includes for the project exXmodemSerialTx here

#include "pifComm.h"

//end of add your includes here


//add your function definitions for the project exXmodemSerialTx here

void actLogPrint(char *pcString);
void actLedLState(PIF_usId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PIF_usId usPifId);
uint16_t actXmodemSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actXmodemReceiveData(PIF_stComm *pstComm, uint8_t *pucData);

//Do not add code below this line
#endif /* _exXmodemSerialTx_H_ */
