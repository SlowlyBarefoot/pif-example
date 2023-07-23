#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "display/pif_dot_matrix.h"


extern PifDotMatrix g_dot_matrix;
extern PifTimerManager g_timer_1ms;


BOOL appSetup();


#endif	// APP_MAIN_H
