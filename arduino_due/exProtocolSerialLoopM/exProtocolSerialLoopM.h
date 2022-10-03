// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialLoopM_H_
#define _exProtocolSerialLoopM_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialLoopM here

#include "core/pif_comm.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exProtocolSerialLoopM here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);
uint16_t actSerial1SendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial1ReceiveData(PifComm *pstComm, uint8_t *pucData);
uint16_t actSerial2SendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial2ReceiveData(PifComm *pstComm, uint8_t *pucData);

//Do not add code below this line
#endif /* _exProtocolSerialLoopM_H_ */
