#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "rc/pif_rc_pwm.h"


extern PifRcPwm g_rc_pwm;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
