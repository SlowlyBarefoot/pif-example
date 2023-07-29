#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"


extern PifLed g_led_l;
extern PifLed g_led_rgb;
extern PifTimerManager g_timer1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
