#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"
#include "pif_comm.h"


extern PifTimerManager *g_pstTimer1ms;
extern PifComm *g_pstCommLog;


void appSetup();


#endif	// APP_MAIN_H
