#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "sensor/pif_hc_sr04.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1


extern PifHcSr04 g_hcsr04;
extern PifTimerManager g_timer_1ms;


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
