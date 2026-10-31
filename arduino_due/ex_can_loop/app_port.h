#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_can.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"


#define TASK_SIZE				12
#define TIMER_1MS_SIZE			2

// CAN0 and CAN1 are joined by two transceivers on one bus. CAN0 sends with CAN_ID_0_TO_1 and CAN1 sends the
// frame back with CAN_ID_1_TO_0.
#define CAN_ID_0_TO_1			0x101
#define CAN_ID_1_TO_0			0x102


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifCan g_can0;
extern PifCan g_can1;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
