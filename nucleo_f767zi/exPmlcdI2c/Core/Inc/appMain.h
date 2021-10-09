#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pifComm.h"
#include "pifPmlcdI2c.h"
#include "pifPulse.h"


extern PIF_stComm *g_pstCommLog;
extern PIF_stPmlcdI2c *g_pstPmlcdI2c;
extern PIF_stPulse *g_pstTimer1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
