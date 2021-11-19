#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_sensor_switch.h"
#include "pif_timer.h"


extern PifSensorSwitch g_push_switch;
extern PifSensorSwitch g_tilt_switch;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H
