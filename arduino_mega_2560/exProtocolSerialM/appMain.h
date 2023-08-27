#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "filter/pif_noise_filter_bit.h"
#include "sensor/pif_sensor_switch.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


typedef struct {
	PifSensorSwitch stPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} ProtocolTest;


extern PifLed g_led_l;
extern PifUart g_serial;
extern PifTimerManager g_timer_1ms;

extern ProtocolTest g_stProtocolTest[SWITCH_COUNT];


BOOL appSetup();


#endif	// APP_MAIN_H
