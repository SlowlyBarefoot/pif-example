// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exFnd4S_H_
#define _exFnd4S_H_
#include "Arduino.h"
//add your includes for the project exFnd4S here

#include "core/pif_comm.h"

//end of add your includes here


//add your function definitions for the project exFnd4S here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit);
uint16_t taskLedToggle(PifTask *pstTask);

//Do not add code below this line
#endif /* _exFnd4S_H_ */
