#ifndef LINKER_H
#define LINKER_H


#include "core/pif_comm.h"
#include "core/pif_timer.h"


extern PifTimerManager g_timer_1ms;


void appSetup();


void actLedLState(PifId pif_id, uint32_t state);
uint16_t actLogSendData(PifComm *p_owner, uint8_t *p_buffer, uint16_t size);
BOOL actLogReceiveData(PifComm *p_owner, uint8_t *p_data);


#endif	// LINKER_H
