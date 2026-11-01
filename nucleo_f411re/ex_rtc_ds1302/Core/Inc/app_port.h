#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "rtc/pif_ds1302.h"


#define TASK_SIZE				8
#define TIMER_1MS_SIZE			1


extern PifLed g_led_l;
extern PifDs1302 g_ds1302;
extern PifTimerManager g_timer_1ms;


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
