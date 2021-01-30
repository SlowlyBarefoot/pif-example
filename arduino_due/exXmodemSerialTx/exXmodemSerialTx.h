// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exXmodemSerialTx_H_
#define _exXmodemSerialTx_H_
#include "Arduino.h"
//add your includes for the project exXmodemSerialTx here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exXmodemSerialTx here

void actLogPrint(char *pcString);
void taskXmodemTest(PIF_stTask *pstTask);
void taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exXmodemSerialTx_H_ */
