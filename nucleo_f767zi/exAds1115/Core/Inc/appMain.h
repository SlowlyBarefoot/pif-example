#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifAds1x1x.h"
#include "pifComm.h"
#include "pifPulse.h"


extern PifAds1x1x *g_pstAds1x1x;
extern PifComm *g_pstCommLog;
extern PifPulse *g_pstTimer1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
