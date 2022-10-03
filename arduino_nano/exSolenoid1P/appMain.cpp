#include "appMain.h"
#include "exSolenoid1P.h"

#include "core/pif_log.h"


typedef struct {
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifTimerManager g_timer_1ms;

static PifSolenoid s_solenoid;
static ST_SolenoidTest s_stSolenoidTest = { &s_solenoid, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

	pifSolenoid_ActionOn(pstParam->pstSolenoid, 0);

	pifTimer_Start(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish()");
}

void appSetup()
{
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifSolenoid_Init(&s_solenoid, PIF_ID_AUTO, &g_timer_1ms,
    		ST_1POINT, 500, actSolenoidOrder)) return;								// 500ms

    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);							// 1000ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
