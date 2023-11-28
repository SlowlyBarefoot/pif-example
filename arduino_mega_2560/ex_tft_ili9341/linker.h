#ifndef LINKER_H
#define LINKER_H


#include "core/pif_log.h"
#include "display/pif_led.h"
#include "display/pif_ili9341.h"
#include "input/pif_touch_screen.h"

#define LCD_2_2_INCH_SPI		0
#define LCD_2_4_INCH			1

#define LCD_TYPE				LCD_2_2_INCH_SPI
//#define LCD_TYPE				LCD_2_4_INCH


extern PifLed g_led_l;
extern PifIli9341 g_ili9341;
extern PifTimerManager g_timer_1ms;
#if LCD_TYPE == LCD_2_4_INCH
	extern PifTouchScreen g_touch_screen;
#endif


BOOL appSetup();


#endif	// LINKER_H
