#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_uart.h"
#include "core/pif_timer.h"


extern PifTimerManager g_timer1ms;
extern PifUart g_uart_log;


void appSetup();


#endif	// APP_MAIN_H
