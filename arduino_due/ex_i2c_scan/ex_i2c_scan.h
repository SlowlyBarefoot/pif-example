// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _ex_i2c_scan_H_
#define _ex_i2c_scan_H_
#include "Arduino.h"
//add your includes for the project ex_i2c_scan here

#include "core/pif_comm.h"
#include "core/pif_i2c.h"

//end of add your includes here

#define USE_I2C_WIRE

//add your function definitions for the project ex_i2c_scan here

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

//Do not add code below this line
#endif /* _ex_i2c_scan_H_ */
