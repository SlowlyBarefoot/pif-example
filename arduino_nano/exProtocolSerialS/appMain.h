#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"


#define PIF_ID_SWITCH			0x100


extern PifTimerManager *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
