#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "motor/pif_step_motor.h"


#define TASK_SIZE				6
#define TIMER_1MS_SIZE			1
#define TIMER_200US_SIZE		1


extern PifLed g_led_l;
extern PifStepMotor g_motor;
extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_200us;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
