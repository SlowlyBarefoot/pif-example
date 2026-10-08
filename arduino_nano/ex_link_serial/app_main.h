#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


extern PifLed g_led_l;
extern PifUart g_serial;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
