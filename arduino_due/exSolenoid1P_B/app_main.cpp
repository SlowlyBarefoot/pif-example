#include "app_port.h"


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

	pifSolenoid_ActionOn(pstParam->pstSolenoid, 0, 2500);	// 2500 * 1ms = 2.5sec

	pifTimer_Start(pstParam->pstTimerItem, 1000);	    	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish()");
}

BOOL appSetup()
{
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 0, 4)) goto fail;

    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) goto fail;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest, FALSE);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);	    // 1000ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exSolenoid1P_B          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
