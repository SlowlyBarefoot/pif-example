#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"


#define SEQUENCE_COUNT          2

#define PIF_ID_LED				0x100
#define PIF_ID_SEQUENCE			0x110
#define PIF_ID_SWITCH			0x120


extern PIF_stPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
