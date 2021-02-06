#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPulse.h"


#define SWITCH_COUNT          	2

#define PIF_ID_LED				0x100
#define PIF_ID_SWITCH			0x110


extern PIF_stComm *g_pstI2C;
extern PIF_stPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
