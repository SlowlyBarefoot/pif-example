#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "sensor/pif_encoder.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1

// 1: drive the encoder input from the quadrature generator, 0: read a real encoder.
#define USE_GENERATOR			1

#define ENCODER_RESOLUTION		ENCODER_RES_X4
#define ENCODER_TIMEOUT_US		500000UL		// The speed reads as stopped after 0.5s without a step


extern PifEncoder g_encoder;
extern PifTimerManager g_timer_1ms;


void actGenerate(uint8_t state);

BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
