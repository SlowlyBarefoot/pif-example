#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_gpio.h"
#include "core/pif_log.h"


extern PifGpio g_gpio_l;
extern PifGpio g_gpio_rg;
extern PifGpio g_gpio_switch;


BOOL appSetup();


#endif	// APP_MAIN_H
