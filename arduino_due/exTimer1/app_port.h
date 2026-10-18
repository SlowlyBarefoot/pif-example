#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_timer_manager.h"


#define TASK_SIZE				1
#define TIMER_1MS_SIZE			1


extern PifTimerManager g_timer_1ms;
extern PifTimer* g_timer_led;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
