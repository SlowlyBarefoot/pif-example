// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exGpsSerialNmea_H_
#define _exGpsSerialNmea_H_
#include "Arduino.h"
//add your includes for the project exGpsSerialNmea here

#include "pif_comm.h"

//end of add your includes here


//add your function definitions for the project exGpsSerialNmea here

void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actGpsSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actGpsReceiveData(PifComm *pstOwner, uint8_t *pucData);
uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exGpsSerialNmea_H_ */
