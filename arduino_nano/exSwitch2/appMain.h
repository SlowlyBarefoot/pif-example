#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifSensorSwitch.h"


extern PIF_stSensor *g_pstPushSwitch;
extern PIF_stSensor *g_pstTiltSwitch;
extern PIF_stPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
