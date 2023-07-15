// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exProtocolSerialLoopS_H_
#define _exProtocolSerialLoopS_H_
#include "Arduino.h"
//add your includes for the project exProtocolSerialLoopS here

#include "communication/pif_uart.h"
#include "sensor/pif_sensor.h"

//end of add your includes here

//#define USE_SERIAL
#define USE_USART

//add your function definitions for the project exProtocolSerialLoopS here

#ifdef USE_SERIAL
uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
#endif
#ifdef USE_USART
BOOL actLogStartTransfer(PifUart* p_uart);
#endif
void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);
#ifdef USE_SERIAL
uint16_t actSerial1SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial1ReceiveData(PifUart *p_uart, uint8_t *pucData);
uint16_t actSerial2SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize);
BOOL actSerial2ReceiveData(PifUart *p_uart, uint8_t *pucData);
#endif
#ifdef USE_USART
BOOL actUart1StartTransfer(PifUart* p_uart);
BOOL actUart2StartTransfer(PifUart* p_uart);
#endif

//Do not add code below this line
#endif /* _exProtocolSerialLoopS_H_ */
