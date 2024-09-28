#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "display/pif_led.h"


extern PifI2cPort g_i2c_port;
extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;


#ifdef __cplusplus
extern "C" {
#endif

BOOL appSetup();

#ifdef __cplusplus
}
#endif


#endif	// APP_MAIN_H
