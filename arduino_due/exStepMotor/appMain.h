#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"


extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stPulse *g_pstTimer200us;


void appSetup(PIF_actTimer1us actTimer1us);


#endif	// APP_MAIN_H
