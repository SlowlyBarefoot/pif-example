#ifndef APPMAIN_H_
#define APPMAIN_H_


#include "communication/pif_uart.h"
#include "core/pif_log.h"
#include "core/pif_timer.h"


extern PifUart s_serial;
extern PifTimerManager g_timer_1ms;


BOOL appInit();
void appExit();


#endif /* APPMAIN_H_ */
