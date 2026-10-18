#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "storage/pif_flash.h"


#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


#define FLASH_RECORD_COUNT		2			// Sectors 6 and 7


extern PifFlash g_flash[FLASH_RECORD_COUNT];
extern PifTimerManager g_timer_1ms;

extern PifTimer* g_timer_led;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
