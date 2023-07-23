#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "display/pif_fnd.h"


extern PifFnd g_fnd;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
