#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"


extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_200us;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
