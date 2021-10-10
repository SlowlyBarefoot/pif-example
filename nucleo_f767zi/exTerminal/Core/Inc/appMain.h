#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


extern PifPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstCommLog;


void appSetup();


#endif	// APP_MAIN_H
