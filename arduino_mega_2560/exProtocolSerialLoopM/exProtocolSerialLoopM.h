// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialLoopM_H_
#define _exProtocolSerialLoopM_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialLoopM here

#include "pifComm.h"

//end of add your includes here

//#define USE_SERIAL
#define USE_USART

//add your function definitions for the project exProtocolSerialLoopM here

#ifdef USE_SERIAL
uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
#endif
#ifdef USE_USART
BOOL actLogStartTransfer();
#endif
void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifId usPifId);
#ifdef USE_SERIAL
uint16_t actSerial1SendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial1ReceiveData(PIF_stComm *pstComm, uint8_t *pucData);
uint16_t actSerial2SendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial2ReceiveData(PIF_stComm *pstComm, uint8_t *pucData);
#endif
#ifdef USE_USART
BOOL actUart1StartTransfer();
BOOL actUart2StartTransfer();
#endif

//Do not add code below this line
#endif /* _exProtocolSerialLoopM_H_ */
