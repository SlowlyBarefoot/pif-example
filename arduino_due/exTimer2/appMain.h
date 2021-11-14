#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_timer.h"


extern PifTimerManager *g_pstTimer1ms;
extern PifTimerManager *g_pstTimer100us;


void appSetup();


#endif	// APP_MAIN_H
