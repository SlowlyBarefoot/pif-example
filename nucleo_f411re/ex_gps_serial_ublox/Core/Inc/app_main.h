#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_comm.h"
#include "core/pif_timer.h"


extern PifComm g_comm_log;
extern PifComm g_comm_gps;
extern PifTimerManager g_timer_1ms;
extern int g_print_data;


void appSetup(uint32_t baurdate);


#endif	// APP_MAIN_H
