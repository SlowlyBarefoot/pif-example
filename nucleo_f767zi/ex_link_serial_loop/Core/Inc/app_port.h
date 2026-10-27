#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "protocol/pif_link.h"

#include "ex_serial1.h"
#include "ex_serial2.h"


#define TASK_SIZE				9
#define TIMER_1MS_SIZE			6


//#define PRINT_PACKET_DATA


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
