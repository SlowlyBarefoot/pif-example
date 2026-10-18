#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "sensor/pif_sensor_threshold.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			3


extern PifSensorThreshold g_sensor;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
