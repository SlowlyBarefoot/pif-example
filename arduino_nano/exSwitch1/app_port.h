#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_gpio.h"
#include "sensor/pif_sensor_switch.h"


#define TASK_SIZE				5


extern PifGpio g_gpio;
extern PifSensorSwitch g_push_switch;
extern PifSensorSwitch g_tilt_switch;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
