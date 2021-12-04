#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_comm.h"
#include "pif_i2c.h"
#include "pif_timer.h"


extern PifComm g_comm_log;
extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
