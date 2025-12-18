#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer_manager.h"


extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_100us;

extern PifTimer* g_timer_red;
extern PifTimer* g_timer_yellow;


BOOL appSetup();


#endif	// APP_MAIN_H
