#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_duty_motor.h"
#include "pif_pulse.h"


#define PIF_ID_SWITCH		0x100


extern PifDutyMotor *g_pstMotor;
extern PifPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
