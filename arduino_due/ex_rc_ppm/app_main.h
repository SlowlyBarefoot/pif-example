#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"
#include "rc/pif_rc_ppm.h"


extern PifRcPpm g_rc_ppm;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
