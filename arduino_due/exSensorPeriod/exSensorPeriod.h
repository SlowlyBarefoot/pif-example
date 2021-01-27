// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exSensorPeriod_H_
#define _exSensorPeriod_H_
#include "Arduino.h"
//add your includes for the project exSensorPeriod here

#include "pifTask.h"

//end of add your includes here


//add your function definitions for the project exSensorPeriod here

void actLogPrint(char *pcString);
void taskLedToggle(PIF_stTask *pstTask);
void taskSensorAcquisition(PIF_stTask *pstTask);

//Do not add code below this line
#endif /* _exSensorPeriod_H_ */
