#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"


#define SWITCH_COUNT          	2

#define PIF_ID_SWITCH			0x100


extern PIF_stPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
