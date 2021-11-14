#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"


#define SEQUENCE_COUNT          2

#define PIF_ID_LED				0x100
#define PIF_ID_SEQUENCE			0x110
#define PIF_ID_SWITCH			0x120


extern PifTimerManager *g_pstTimer1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
