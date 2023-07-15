#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_uart.h"
#include "communication/pif_i2c.h"
#include "core/pif_timer.h"


extern PifUart g_uart_log;
extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H
