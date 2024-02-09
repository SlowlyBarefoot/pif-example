#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "display/pif_ssd1963.h"
#include "input/pif_tsc2046.h"


extern PifLed g_led_l;
extern PifSsd1963 g_ssd1963;
extern PifTimerManager g_timer_1ms;
extern PifSpiPort g_spi_port;
extern PifTsc2046 g_tsc2046;


BOOL appSetup();


#endif	// LINKER_H
