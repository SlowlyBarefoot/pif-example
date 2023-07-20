#ifndef LINKER_H
#define LINKER_H


#include "communication/pif_uart.h"
#include "communication/pif_i2c.h"
#include "core/pif_timer.h"


extern PifUart g_uart_log;
extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;

extern int g_print_data;


void actLedLState(PifId pif_id, uint32_t new_state);
BOOL actLogStartTransfer(PifUart* p_uart);
PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);
PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size);

void appSetup();


#endif	// LINKER_H