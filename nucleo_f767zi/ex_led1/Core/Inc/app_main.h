#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_comm.h"
#include "core/pif_timer.h"


extern PifTimerManager g_timer1ms;
extern PifComm g_comm_log;


void appSetup();


#endif	// APP_MAIN_H
