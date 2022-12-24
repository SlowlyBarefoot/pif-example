#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"
#include "rc/pif_rc_pwm.h"


extern PifRcPwm g_rc_pwm;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
