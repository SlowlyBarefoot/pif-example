#ifndef APPMAIN_H_
#define APPMAIN_H_


#include "pifComm.h"
#include "pifPulse.h"


extern PIF_stPulse *g_pstTimer1ms;
extern PIF_stComm *g_pstSerial;


BOOL appInit();
void appExit();


#endif /* APPMAIN_H_ */
