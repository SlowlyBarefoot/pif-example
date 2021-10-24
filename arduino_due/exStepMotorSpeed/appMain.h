#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_pulse.h"


#define PIF_ID_SWITCH		0x100


extern PifPulse *g_pstTimer1ms;
extern PifPulse *g_pstTimer200us;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
