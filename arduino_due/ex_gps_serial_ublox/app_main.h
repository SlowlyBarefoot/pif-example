#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "display/pif_led.h"


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_gps;

extern int g_print_data;


BOOL appSetup(uint32_t baurate);


#endif	// APP_MAIN_H
