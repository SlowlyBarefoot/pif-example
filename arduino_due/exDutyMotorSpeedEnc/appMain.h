#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifDutyMotor.h"
#include "pifPulse.h"


#define PIF_ID_SWITCH		0x100


extern PIF_stDutyMotor *g_pstMotor;
extern PifPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
