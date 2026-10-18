#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "protocol/pif_link.h"
#include "sensor/pif_sensor_switch.h"


#define TASK_SIZE				9
#define TIMER_1MS_SIZE			6

#define PIF_ID_SWITCH			0x100

#define SWITCH_COUNT          	2


typedef struct {
	PifSensorSwitch push_switch;
	uint8_t data_count;
	uint8_t data[8];
} LinkTest;


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifUart g_serial1;
extern PifUart g_serial2;
extern LinkTest g_link_test[SWITCH_COUNT];


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
