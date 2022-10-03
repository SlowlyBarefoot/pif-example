// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSolenoid1P_B_H_
#define _exSolenoid1P_B_H_
#include "Arduino.h"
//add your includes for the project exSolenoid1P_B here

#include "actulator/pif_solenoid.h"
#include "core/pif_comm.h"

//end of add your includes here


//add your function definitions for the project exSolenoid1P_B here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actSolenoidOrder(SWITCH swOrder, PifSolenoidDir enDir);
uint16_t taskLedToggle(PifTask *pstTask);

//Do not add code below this line
#endif /* _exSolenoid1P_B_H_ */
