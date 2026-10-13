#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "communication/pif_can_stream.h"
#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "display/pif_led.h"
#include "protocol/pif_link.h"

#include "ex_can0.h"
#include "ex_can1.h"


// CAN0 and CAN1 are joined by two transceivers on one bus. Each link uses one ID per direction.
#define CAN_ID_0_TO_1			0x101
#define CAN_ID_1_TO_0			0x102

// Bytes a stream holds until its link reads them: more than the longest link frame.
#define CAN_STREAM_RX_SIZE		128


extern PifLed g_led_l;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
