#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


extern PifLed g_led_l;
extern PifLed g_led_ry;
extern PifSensorSwitch g_push_switch;
extern PifSensorSwitch g_tilt_switch;
extern PifTimerManager g_timer_1ms;

extern PifTimer* g_timer_switch;


BOOL appSetup();


#endif	// APP_MAIN_H
