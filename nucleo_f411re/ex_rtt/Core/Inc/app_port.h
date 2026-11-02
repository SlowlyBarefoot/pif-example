#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_rtt.h"
#include "core/pif_log.h"
#include "display/pif_led.h"


#define TASK_SIZE				8


extern PifLed g_led_l;
extern PifRtt g_rtt;				// RTT channel 0: log and command line


uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
