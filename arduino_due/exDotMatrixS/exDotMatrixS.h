// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exDotMatrixS_H_
#define _exDotMatrixS_H_
#include "Arduino.h"
//add your includes for the project exDotMatrixS here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exDotMatrixS here

void actLogPrint(char *pcString);
void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData);
uint16_t taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exDotMatrixS_H_ */
