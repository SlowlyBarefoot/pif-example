// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exTerminal_H_
#define _exTerminal_H_
#include "Arduino.h"
//add your includes for the project exTerminal here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exTerminal here

void actLogPrint(char *pcString);
uint16_t taskTerminal(PIF_stTask *pstTask);
uint16_t taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exTerminal_H_ */
