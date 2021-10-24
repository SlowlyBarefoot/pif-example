// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exTerminal_H_
#define _exTerminal_H_
#include "Arduino.h"
//add your includes for the project exTerminal here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exTerminal here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
#ifdef __PIF_LOG_COMMAND__
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);
#endif
void actLedLState(PifId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exTerminal_H_ */
