// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSolenoid1P_2_H_
#define _exSolenoid1P_2_H_
#include "Arduino.h"
//add your includes for the project exSolenoid1P_2 here

#include "pifSolenoid.h"

//end of add your includes here


//add your function definitions for the project exSolenoid1P_2 here

void actLogPrint(char *pcString);
void actSolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir);
uint16_t taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exSolenoid1P_2_H_ */
