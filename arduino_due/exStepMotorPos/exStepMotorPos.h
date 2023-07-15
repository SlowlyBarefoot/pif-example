// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exStepMotorPos_H_
#define _exStepMotorPos_H_
#include "Arduino.h"
//add your includes for the project exStepMotorPos here

#include "communication/pif_uart.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exStepMotorPos here

uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifUart *pstOwner, uint8_t *pucData);
uint16_t actPhotoInterruptAcquire(PifSensor* p_owner);
void actSetStep(uint16_t usPhase);
void actLedLState(PifId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exStepMotorPos_H_ */
