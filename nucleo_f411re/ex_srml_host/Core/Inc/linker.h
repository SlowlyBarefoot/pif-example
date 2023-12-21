#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_printer;


BOOL appSetup();


#endif	// LINKER_H
