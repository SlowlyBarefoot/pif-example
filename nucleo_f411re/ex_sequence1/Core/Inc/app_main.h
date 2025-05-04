#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer.h"


#define TASK_SIZE				6
#define TIMER_1MS_SIZE			1


extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
