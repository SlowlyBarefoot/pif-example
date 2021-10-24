#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_pulse.h"

#include "exSerial1.h"
#include "exSerial2.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


extern PifPulse *g_pstTimer1ms;
extern PifComm *g_pstCommLog;


void appSetup();


#endif	// APP_MAIN_H
