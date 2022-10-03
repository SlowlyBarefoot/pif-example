#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"
#include "sensor/pif_hc_sr04.h"


extern PifHcSr04 g_hcsr04;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
