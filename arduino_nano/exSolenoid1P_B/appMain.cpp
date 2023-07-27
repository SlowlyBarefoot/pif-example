#include "appMain.h"


typedef struct {
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifSolenoid g_solenoid;
PifTimerManager g_timer_1ms;

static ST_SolenoidTest s_stSolenoidTest = {	&g_solenoid, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

	pifSolenoid_ActionOn(pstParam->pstSolenoid, 2500);	// 2500 * 1ms = 2.5sec

	pifTimer_Start(pstParam->pstTimerItem, 1000);	    // 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish()");
}

BOOL appSetup()
{
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return FALSE;

    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return FALSE;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);	    // 1000ms
    return TRUE;
}
