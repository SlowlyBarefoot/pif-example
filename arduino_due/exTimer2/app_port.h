#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_timer_manager.h"


#define TASK_SIZE				2
#define TIMER_1MS_SIZE			1
#define TIMER_100US_SIZE		1


extern PifTimerManager g_timer_1ms;


BOOL portSetup();


#endif	// APP_PORT_H
