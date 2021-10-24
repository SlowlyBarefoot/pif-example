#ifndef APP_MAIN_H
#define APP_MAIN_H


#include "pif_comm.h"
#include "pif_pmlcd_i2c.h"
#include "pif_pulse.h"


extern PifComm *g_pstCommLog;
extern PifPmlcdI2c *g_pstPmlcdI2c;
extern PifPulse *g_pstTimer1ms;


void appSetup(PifActTimer1us act_timer1us);


#endif	// APP_MAIN_H
