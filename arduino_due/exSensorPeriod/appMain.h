#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "sensor/pif_sensor_digital.h"


extern PifSensorDigital g_sensor;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
