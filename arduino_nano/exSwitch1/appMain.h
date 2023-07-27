#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_gpio.h"
#include "sensor/pif_sensor_switch.h"


extern PifGpio g_gpio;
extern PifSensorSwitch g_push_switch;
extern PifSensorSwitch g_tilt_switch;


BOOL appSetup();


#endif	// APP_MAIN_H
