#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"

#include "exSerial1.h"
#include "exSerial2.h"


//#define PRINT_PACKET_DATA

#define PIF_ID_SWITCH			0x100


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
