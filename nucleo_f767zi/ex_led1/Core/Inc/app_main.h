#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_pulse.h"
#include "pif_comm.h"


extern PifPulse g_timer1ms;
extern PifComm g_comm_log;


void appSetup();


#endif	// APP_MAIN_H
