#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"


extern PifI2cPort g_i2c_port;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
