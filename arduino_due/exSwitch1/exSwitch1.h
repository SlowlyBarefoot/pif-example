// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSwitch1_H_
#define _exSwitch1_H_
#include "Arduino.h"
//add your includes for the project exSwitch1 here

#include "core/pif_comm.h"
#include "sensor/pif_sensor.h"

//end of add your includes here


//add your function definitions for the project exSwitch1 here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t actPushSwitchAcquire(PifSensor* p_owner);
void evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer);
uint16_t actTiltSwitchAcquire(PifSensor* p_owner);
void evtTiltSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer);
uint16_t taskLedToggle(PifTask *pstTask);

//Do not add code below this line
#endif /* _exSwitch1_H_ */
