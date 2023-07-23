#include "appMain.h"


typedef struct {
	PifSolenoidDir enDir;
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifSolenoid g_solenoid;
PifTimerManager g_timer_1ms;

static ST_SolenoidTest s_stSolenoidTest = { SD_LEFT, &g_solenoid, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 0, pstParam->enDir);

	pifTimer_Start(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_RIGHT ? SD_LEFT : SD_RIGHT;
}

BOOL appSetup()
{
    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return FALSE;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);					    	// 1000ms
    return TRUE;
}
