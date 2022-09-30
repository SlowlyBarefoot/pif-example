// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_storage_var_i2c_H_
#define _ex_storage_var_i2c_H_
#include "Arduino.h"
//add your includes for the project ex_storage_var_i2c here

#include "pif_comm.h"
#include "pif_i2c.h"

//end of add your includes here

//#define USE_I2C_WIRE

//add your function definitions for the project ex_storage_var_i2c here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData);
void actLedL(SWITCH sw);
PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

//Do not add code below this line
#endif /* _ex_storage_var_i2c_H_ */
