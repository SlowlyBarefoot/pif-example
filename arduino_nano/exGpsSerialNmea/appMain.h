#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


extern PifLed g_led_l;
extern PifSensorSwitch g_push_switch;
extern PifTimerManager g_timer_1ms;
extern PifUart g_uart_gps;


BOOL appSetup();


#endif	// APP_MAIN_H
