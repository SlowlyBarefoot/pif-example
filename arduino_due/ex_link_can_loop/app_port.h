#ifndef APP_PORT_H
#define APP_PORT_H


#include "communication/pif_can.h"
#include "communication/pif_can_stream.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "protocol/pif_link.h"
#include "sensor/pif_sensor_switch.h"


#define TASK_SIZE				12
#define TIMER_1MS_SIZE			6

#define PIF_ID_SWITCH			0x100

#define SWITCH_COUNT          	2

// CAN0 and CAN1 are joined by two transceivers on one bus. Each link uses one ID per direction.
#define CAN_ID_0_TO_1			0x101
#define CAN_ID_1_TO_0			0x102

// Bytes a stream holds until its link reads them: more than the longest link frame.
#define CAN_STREAM_RX_SIZE		128


typedef struct {
	PifSensorSwitch push_switch;
	uint8_t data_count;
	uint8_t data[8];
} LinkTest;


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;
extern PifCan g_can0;
extern PifCan g_can1;
extern LinkTest g_link_test[SWITCH_COUNT];


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
