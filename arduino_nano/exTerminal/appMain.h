#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstCommLog;


void appSetup(PIF_actTimer1us actTimer1us);


#endif	// APP_MAIN_H
