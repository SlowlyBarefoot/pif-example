// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exDutyMotor_H_
#define _exDutyMotor_H_
#include "Arduino.h"
//add your includes for the project exDutyMotor here

#include "communication/pif_uart.h"

//end of add your includes here


//add your function definitions for the project exDutyMotor here

uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifUart *pstOwner, uint8_t *pucData);
void actSetDuty(uint16_t usDuty);
void actSetDirection(uint8_t ucDir);
void actOperateBreak(uint8_t ucState);
void actLedLState(PifId usPifId, uint32_t unState);

//Do not add code below this line
#endif /* _exDutyMotor_H_ */
