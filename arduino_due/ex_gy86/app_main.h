#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "display/pif_led.h"


//#define USE_I2C_WIRE


extern PifI2cPort g_i2c_port;
extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
