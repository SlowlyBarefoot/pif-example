#ifndef APPMAIN_H_
#define APPMAIN_H_


#include "pif_timer.h"


extern PifTimerManager g_timer_1ms;


BOOL appInit();
void appExit();


#endif /* APPMAIN_H_ */
