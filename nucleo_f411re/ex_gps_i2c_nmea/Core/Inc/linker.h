#ifndef LINKER_H
#define LINKER_H


#include "communication/pif_i2c.h"
#include "core/pif_log.h"
#include "display/pif_led.h"


extern PifI2cPort g_i2c_port;
extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;

extern int g_print_data;


BOOL appSetup();


#endif	// LINKER_H
