// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _exAds1115_H_
#define _exAds1115_H_
#include "Arduino.h"
//add your includes for the project exAds1115 here

#include "communication/pif_uart.h"
#include "communication/pif_i2c.h"

//end of add your includes here

//#define USE_I2C_WIRE

//add your function definitions for the project exAds1115 here

uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
void actLedLState(PifId usPifId, uint32_t unState);
PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

//Do not add code below this line
#endif /* _exAds1115_H_ */
