#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifSensorSwitch.h"


extern PifSensor *g_pstPushSwitch;
extern PifSensor *g_pstTiltSwitch;
extern PifPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
