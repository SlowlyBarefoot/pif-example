#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "rc/pif_rc_pwm.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


extern PifRcPwm g_rc_pwm;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
