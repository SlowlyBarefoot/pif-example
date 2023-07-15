#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_uart.h"
#include "core/pif_timer.h"


extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_log;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
