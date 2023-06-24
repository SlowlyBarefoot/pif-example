// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_gy86_H_
#define _ex_gy86_H_
#include "Arduino.h"
//add your includes for the project ex_gy86 here

#include "core/pif_comm.h"
#include "core/pif_i2c.h"

//end of add your includes here

//#define USE_I2C_WIRE

//add your function definitions for the project ex_gy86 here

uint16_t actLogSendData(PifComm* p_cwner, uint8_t* p_buffer, uint16_t size);
void actLedLState(PifId id, uint32_t state);
PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

//Do not add code below this line
#endif /* _ex_gy86_H_ */
