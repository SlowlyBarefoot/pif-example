// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exCollectSignal2_H_
#define _exCollectSignal2_H_
#include "Arduino.h"
//add your includes for the project exCollectSignal2 here

#include "communication/pif_uart.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exCollectSignal2 here

void actLedLState(PifId usPifId, uint32_t unState);
void actGpioRGBState(PifId usPifId, uint8_t unState);
void actLedCollectState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);
uint16_t actPushSwitchCollectAcquire(PifSensor* p_owner);
uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifUart *pstOwner, uint8_t *pucData);

//Do not add code below this line
#endif /* _exCollectSignal2_H_ */
