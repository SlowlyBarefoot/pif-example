#ifndef APP_PORT_H
#define APP_PORT_H


#include "core/pif_log.h"
#include "core/pif_task_manager.h"


// realtime, control, event, backup, dump, and the log command, receiver and transmitter
#define TASK_SIZE				8


// Drives a pin while the realtime task runs, to watch its jitter on a scope.
void actRealtimeSignal(BOOL state);

uint32_t actMicros();
BOOL portSetup();
BOOL appSetup();


#endif	// APP_PORT_H
