#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_pulse.h"
#include "core/pif_timer.h"


extern PifPulse g_pulse;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
