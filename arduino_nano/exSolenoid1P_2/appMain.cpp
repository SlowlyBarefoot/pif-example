#include "appMain.h"


typedef struct {
	SWITCH nSwitch;
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifSolenoid g_solenoid;
PifTimerManager g_timer_1ms;

static ST_SolenoidTest s_stSolenoidTest = {	OFF, &g_solenoid, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

	if (pstParam->nSwitch) {
		pifSolenoid_ActionOn(pstParam->pstSolenoid, 0);
	}
	else {
		pifSolenoid_ActionOff(pstParam->pstSolenoid);
	}

	pifTimer_Start(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->nSwitch);

	pstParam->nSwitch ^= 1;
}

BOOL appSetup()
{
    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return FALSE;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);							// 1000ms
    return TRUE;
}
