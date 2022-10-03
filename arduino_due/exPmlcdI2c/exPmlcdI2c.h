// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exPmlcdI2c_H_
#define _exPmlcdI2c_H_
#include "Arduino.h"
//add your includes for the project exPmlcdI2c here

#include "core/pif_comm.h"
#include "core/pif_i2c.h"

//end of add your includes here


//add your function definitions for the project exPmlcdI2c here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

//Do not add code below this line
#endif /* _exPmlcdI2c_H_ */
