#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "motor/pif_step_motor_pos.h"
#include "sensor/pif_sensor_switch.h"


#define SWITCH_COUNT        3

#define PIF_ID_SWITCH		0x100


extern PifLed g_led_l;
extern PifStepMotorPos g_motor;
extern PifSensorSwitch g_switch[SWITCH_COUNT];
extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_200us;


BOOL appSetup();


#endif	// APP_MAIN_H
