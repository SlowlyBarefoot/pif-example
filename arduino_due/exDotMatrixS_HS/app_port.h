#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_timer_manager.h"
#include "core/pif_timer.h"
#include "display/pif_dot_matrix.h"


#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2


extern PifDotMatrix g_dot_matrix;
extern PifTimerManager g_timer_1ms;


BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
