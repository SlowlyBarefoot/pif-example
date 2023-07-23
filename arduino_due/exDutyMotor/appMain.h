#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "motor/pif_duty_motor.h"


extern PifLed g_led_l;
extern PifDutyMotor g_motor;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
