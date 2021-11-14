#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"
#include "pif_comm.h"


extern PifTimerManager g_timer1ms;
extern PifComm g_comm_log;


void appSetup();


#endif	// APP_MAIN_H
