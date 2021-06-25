// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exFnd1S_SR_H_
#define _exFnd1S_SR_H_
#include "Arduino.h"
//add your includes for the project exFnd1S_SR here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exFnd1S_SR here

void actLogPrint(char *pcString);
void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit);
uint16_t taskLedToggle(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exFnd1S_SR_H_ */
