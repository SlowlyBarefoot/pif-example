#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstSerial;


void appSetup();


#endif	// APP_MAIN_H
