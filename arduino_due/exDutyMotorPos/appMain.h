#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_duty_motor_pos.h"
#include "pif_timer.h"


#define PIF_ID_SWITCH		0x100


extern PifDutyMotorPos g_motor;
extern PifTimerManager g_timer_1ms;


void appSetup();


#endif	// APP_MAIN_H
