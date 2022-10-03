#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"

#include "exSerial1.h"
#include "exSerial2.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


extern PifTimerManager g_timer_1ms;
extern PifComm g_comm_log;


void appSetup();


#endif	// APP_MAIN_H
