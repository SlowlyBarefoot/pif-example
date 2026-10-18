#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "sensor/pif_pulse.h"
#include "core/pif_timer_manager.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


extern PifPulse g_pulse;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
