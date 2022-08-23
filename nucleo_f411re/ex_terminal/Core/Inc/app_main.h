#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_comm.h"
#include "pif_timer.h"


extern PifTimerManager g_timer_1ms;
extern PifComm g_comm_log;


void appSetup();


#endif	// APP_MAIN_H
