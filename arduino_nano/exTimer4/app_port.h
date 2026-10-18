#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_timer.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				1
#define TIMER_1MS_SIZE			3


extern PifLed g_led_rgb;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
