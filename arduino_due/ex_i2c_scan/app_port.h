#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "core/pif_task_manager.h"
#include "core/pif_timer_manager.h"


#define TASK_SIZE				3
#define TIMER_1MS_SIZE			1


extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
