#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_comm.h"
#include "core/pif_i2c.h"
#include "core/pif_timer.h"


extern PifComm g_comm_log;
extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H