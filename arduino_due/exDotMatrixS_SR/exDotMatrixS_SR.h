// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exDotMatrixS_SR_H_
#define _exDotMatrixS_SR_H_
#include "Arduino.h"
//add your includes for the project exDotMatrixS_SR here

#include "pifPulse.h"

//end of add your includes here


//add your function definitions for the project exDotMatrixS_SR here

extern PIF_stPulse *g_pstTimer1ms;


void actLogPrint(char *pcString);
void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData);
void taskLedToggle(PIF_stTask *pstTask);

void appSetup();

//Do not add code below this line
#endif /* _exDotMatrixS_SR_H_ */
