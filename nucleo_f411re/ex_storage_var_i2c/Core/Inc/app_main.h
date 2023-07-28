#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "storage/pif_storage_var.h"


extern PifI2cPort g_i2c_port;
extern PifStorageVar g_storage;
extern PifTimerManager g_timer_1ms;

extern PifTimer* g_timer_led;


BOOL appSetup();


#endif	// APP_MAIN_H
