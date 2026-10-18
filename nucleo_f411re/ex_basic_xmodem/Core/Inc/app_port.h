#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE		11
#define TIMER_1MS_SIZE	4


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
