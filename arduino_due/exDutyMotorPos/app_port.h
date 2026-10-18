#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "motor/pif_duty_motor_pos.h"
#include "sensor/pif_sensor_switch.h"


#define TASK_SIZE				9
#define TIMER_1MS_SIZE			2

#define SWITCH_COUNT		3

#define PIF_ID_SWITCH		0x100


extern PifLed g_led_l;
extern PifDutyMotorPos g_motor;
extern PifSensorSwitch g_switch[SWITCH_COUNT];
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
