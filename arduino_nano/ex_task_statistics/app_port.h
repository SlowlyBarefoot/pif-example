#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_task_manager.h"


// realtime, control, backup, and the log transmitter
#define TASK_SIZE				4


// Drives a pin while the realtime task runs, to watch its jitter on a scope.
void actRealtimeSignal(BOOL state);

BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
