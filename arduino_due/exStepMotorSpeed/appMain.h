#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"


#define PIF_ID_SWITCH		0x100


extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_200us;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
