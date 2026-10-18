#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1


extern PifLed g_led_l;
extern PifSensorSwitch g_push_switch;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_gps;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
