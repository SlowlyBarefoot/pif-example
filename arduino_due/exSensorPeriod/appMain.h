#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_sensor_digital.h"


extern PifTimerManager g_timer_1ms;
extern PifSensor *g_pstSensor;


void appSetup();


#endif	// APP_MAIN_H
