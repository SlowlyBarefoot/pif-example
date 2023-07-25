#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"


extern PifTimerManager g_timer_1ms;
extern PifTimer* g_timer_led;


BOOL appSetup();


#endif	// APP_MAIN_H
