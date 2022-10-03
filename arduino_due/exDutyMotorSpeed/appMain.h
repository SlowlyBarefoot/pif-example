#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_timer.h"
#include "motor/pif_duty_motor_speed.h"


#define PIF_ID_SWITCH		0x100


extern PifDutyMotorSpeed g_motor;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H
