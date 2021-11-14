#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_comm.h"
#include "pif_timer.h"


extern PifTimerManager *g_pstTimer1ms;
extern PifComm *g_pstCommLog;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
