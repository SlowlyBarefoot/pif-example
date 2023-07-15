#ifndef LINKER_H
#define LINKER_H


#include "communication/pif_uart.h"
#include "communication/pif_i2c.h"
#include "core/pif_timer.h"


//#define USE_I2C_WIRE


extern PifTimerManager g_timer_1ms;


void appSetup();


void actLedLState(PifId usPifId, uint32_t unState);
uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize);
BOOL actLogReceiveData(PifUart *pstOwner, uint8_t *pucData);
PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);


#endif	// LINKER_H
