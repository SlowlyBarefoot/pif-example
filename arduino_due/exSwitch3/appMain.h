#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_sensor_switch.h"
#include "pif_timer.h"


extern PifSensor *g_pstPushSwitch;
extern PifSensor *g_pstTiltSwitch;
extern PifTimerManager *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
