#ifndef APP_PORT_H
#define APP_PORT_H


#include "actuator/pif_solenoid.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


extern PifSolenoid g_solenoid;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
