// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSolenoid1P_B_H_
#define _exSolenoid1P_B_H_
#include "Arduino.h"
//add your includes for the project exSolenoid1P_B here

#include "pifSolenoid.h"

//end of add your includes here


//add your function definitions for the project exSolenoid1P_B here

void actLogPrint(char *pcString);
void actSolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir);
void taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exSolenoid1P_B_H_ */
