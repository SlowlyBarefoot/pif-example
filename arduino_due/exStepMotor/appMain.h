#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "motor/pif_step_motor.h"


extern PifLed g_led_l;
extern PifStepMotor g_motor;
extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_200us;


BOOL appSetup();


#endif	// APP_MAIN_H
