#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "display/pif_ili9341.h"
#include "input/pif_tsc2046.h"


#define TASK_SIZE				11
#define TIMER_1MS_SIZE			2


extern PifLed g_led_l;
extern PifIli9341 g_ili9341;
extern PifTimerManager g_timer_1ms;
extern PifSpiPort g_spi_port;
extern PifTsc2046 g_tsc2046;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
