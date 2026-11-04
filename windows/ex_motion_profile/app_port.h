#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "control/pif_motion_profile.h"
#include "core/pif_timer_manager.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


extern PifTimerManager g_timer_1ms;


void portTick1ms();
void portStepPulse(int8_t dir);
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
