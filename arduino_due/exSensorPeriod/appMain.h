#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"
#include "pifSensor.h"


extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stSensor *g_pstSensor;


void appSetup();


#endif	// APP_MAIN_H
