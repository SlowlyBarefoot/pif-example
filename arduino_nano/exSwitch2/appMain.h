#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifPulse.h"
#include "pifSwitch.h"


extern PIF_stSwitch *g_pstPushSwitch;
extern PIF_stSwitch *g_pstTiltSwitch;
extern PIF_stPulse *g_pstTimer1ms;


void appSetup();


#endif	// APP_MAIN_H
