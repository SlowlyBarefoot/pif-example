#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_sensor_digital.h"


extern PifTimerManager g_timer_1ms;
extern PifSensorDigital g_sensor;


void appSetup();


#endif	// APP_MAIN_H
