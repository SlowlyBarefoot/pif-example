#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


extern PifPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstCommLog;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
