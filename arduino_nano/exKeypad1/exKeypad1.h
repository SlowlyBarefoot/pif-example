// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exKeypad1_H_
#define _exKeypad1_H_
#include "Arduino.h"
//add your includes for the project exKeypad1 here

#include "pif_comm.h"

//end of add your includes here

#define	ROWS	4		// 행(rows) 개수
#define	COLS	4		// 열(columns) 개수

//add your function definitions for the project exKeypad1 here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
uint16_t taskLedToggle(PifTask *pstTask);
void actKeypadAcquire(uint16_t *pusState);

//Do not add code below this line
#endif /* _exKeypad1_H_ */
