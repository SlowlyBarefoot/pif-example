// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exXmodemSerialRx_H_
#define _exXmodemSerialRx_H_
#include "Arduino.h"
//add your includes for the project exXmodemSerialRx here

#include "core/pif_comm.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exXmodemSerialRx here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);
uint16_t actXmodemSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actXmodemReceiveData(PifComm *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exXmodemSerialRx_H_ */
