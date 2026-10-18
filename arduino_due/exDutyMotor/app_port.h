#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "motor/pif_duty_motor.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2


extern PifLed g_led_l;
extern PifDutyMotor g_motor;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
