#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_uart.h"
#include "core/pif_timer.h"


extern PifUart g_uart_log;
extern PifUart g_uart_gps;
extern PifTimerManager g_timer_1ms;
extern int g_print_data;


void appSetup(uint32_t baurdate);


#endif	// APP_MAIN_H
