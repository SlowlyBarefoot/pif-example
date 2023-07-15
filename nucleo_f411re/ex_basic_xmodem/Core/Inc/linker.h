#ifndef LINKER_H
#define LINKER_H


#include "communication/pif_uart.h"
#include "core/pif_timer.h"


extern PifUart g_uart;
extern PifTimerManager g_timer_1ms;


void appSetup();


void actLedLState(PifId pif_id, uint32_t state);
BOOL actLogStartTransfer(PifUart* p_uart);


#endif	// LINKER_H
