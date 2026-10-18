#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE		7
#define TIMER_1MS_SIZE	1


extern PifI2cPort g_i2c_port;
extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;

extern int g_print_data;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
