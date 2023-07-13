#ifndef LINKER_H
#define LINKER_H


#include "core/pif_comm.h"
#include "core/pif_timer.h"


extern PifComm g_comm;
extern PifTimerManager g_timer_1ms;


void appSetup();


void actLedLState(PifId pif_id, uint32_t state);
BOOL actLogStartTransfer(PifComm* p_comm);


#endif	// LINKER_H
