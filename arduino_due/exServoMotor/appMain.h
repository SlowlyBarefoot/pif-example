#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"


extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_100us;


void appSetup();


#endif	// APP_MAIN_H
