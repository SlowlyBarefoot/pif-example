#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_sequence.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


#define SEQUENCE_COUNT          2

#define PIF_ID_LED				0x100
#define PIF_ID_SEQUENCE			0x110
#define PIF_ID_SWITCH			0x120


typedef struct {
	PifSensorSwitch stPushSwitch;
	PifSequence stSequence;
	BOOL bSequenceParam;
} SequenceTest;


extern PifLed g_led_l;
extern PifLed g_led_rgb;
extern PifTimerManager g_timer_1ms;

extern SequenceTest g_stSequenceTest[SEQUENCE_COUNT];


BOOL appSetup();


#endif	// APP_MAIN_H
