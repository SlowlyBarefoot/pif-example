// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSequence2_H_
#define _exSequence2_H_
#include "Arduino.h"
//add your includes for the project exSequence2 here

#include "communication/pif_uart.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exSequence2 here

uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
void actLedRGBState(PifId usPifId, uint32_t unState);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);

//Do not add code below this line
#endif /* _exSequence2_H_ */
