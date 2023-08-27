#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "display/pif_ili9341.h"
#include "input/pif_touch_screen.h"


extern PifLed g_led_l;
extern PifIli9341 g_ili9341;
extern PifTimerManager g_timer_1ms;
extern PifTouchScreen g_touch_screen;


BOOL appSetup();


#endif	// LINKER_H
