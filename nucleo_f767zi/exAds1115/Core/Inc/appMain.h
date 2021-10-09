#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifAds1x1x.h"
#include "pifComm.h"
#include "pifPulse.h"


extern PIF_stAds1x1x *g_pstAds1x1x;
extern PIF_stComm *g_pstCommLog;
extern PIF_stPulse *g_pstTimer1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
