#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_ads1x1x.h"
#include "pif_comm.h"
#include "pif_timer.h"


extern PifAds1x1x g_ads1x1x;
extern PifComm g_comm_log;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
