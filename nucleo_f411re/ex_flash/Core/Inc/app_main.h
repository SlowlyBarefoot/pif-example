#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "storage/pif_flash.h"


#define FLASH_RECORD_COUNT		2			// Sectors 6 and 7


extern PifFlash g_flash[FLASH_RECORD_COUNT];
extern PifTimerManager g_timer_1ms;

extern PifTimer* g_timer_led;


BOOL appSetup();


#endif	// APP_MAIN_H
