#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_pwm_gen.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifTimerManager g_timer_100us;
extern PifPwmGen g_pwm_gen;


BOOL appSetup();


#endif	// APP_MAIN_H
