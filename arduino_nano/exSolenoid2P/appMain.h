#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "actulator/pif_solenoid.h"
#include "core/pif_log.h"


extern PifSolenoid g_solenoid;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
