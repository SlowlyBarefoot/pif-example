#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "sensor/pif_encoder.h"


// 1: drive the encoder input from the quadrature generator, 0: read a real encoder.
#define USE_GENERATOR			1


typedef void (*AppActGenerate)(uint8_t state);


extern PifEncoder g_encoder;
extern PifTimerManager g_timer_1ms;


BOOL appSetup(AppActGenerate act_generate);


#endif	// APP_MAIN_H
