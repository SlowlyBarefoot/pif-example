#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_pwm_gen.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1
#define TIMER_100US_SIZE		1


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_100us;
extern PifPwmGen g_pwm_gen;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
