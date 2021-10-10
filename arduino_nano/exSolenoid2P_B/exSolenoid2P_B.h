// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSolenoid2P_B_H_
#define _exSolenoid2P_B_H_
#include "Arduino.h"
//add your includes for the project exSolenoid2P_B here

#include "pifComm.h"
#include "pifSolenoid.h"

//end of add your includes here


//add your function definitions for the project exSolenoid2P_B here

uint16_t actLogSendData(PIF_stComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actSolenoidOrder(SWITCH swOrder, PifSolenoidDir enDir);
uint16_t taskLedToggle(PifTask *pstTask);

//Do not add code below this line
#endif /* _exSolenoid2P_B_H_ */
