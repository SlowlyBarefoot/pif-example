#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


extern PIF_stComm *g_pstSerial;
extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stPulse *g_pstTimer200us;


void appSetup();


#endif	// APP_MAIN_H
