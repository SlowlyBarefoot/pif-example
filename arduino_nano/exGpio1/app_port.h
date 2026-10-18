#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_gpio.h"
#include "core/pif_log.h"


#define TASK_SIZE				2


extern PifGpio g_gpio_l;
extern PifGpio g_gpio_rg;
extern PifGpio g_gpio_switch;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
