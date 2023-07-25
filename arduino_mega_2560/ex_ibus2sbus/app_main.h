#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "display/pif_led.h"


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_ibus;
extern PifUart g_uart_sbus;


BOOL appSetup();


#endif	// APP_MAIN_H
