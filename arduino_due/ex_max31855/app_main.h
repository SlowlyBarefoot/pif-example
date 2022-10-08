#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_comm.h"
#include "core/pif_timer.h"


extern PifComm g_comm_log;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H
