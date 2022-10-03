#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_pulse.h"
#include "core/pif_timer.h"


#define PIF_ID_SWITCH		0x100


extern PifPulse g_encoder;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
