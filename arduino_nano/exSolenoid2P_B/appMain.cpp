#include "appMain.h"
#include "exSolenoid2P_B.h"

#include "pif_log.h"


typedef struct {
	PifSolenoidDir enDir;
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifTimerManager *g_pstTimer1ms = NULL;

static ST_SolenoidTest s_stSolenoidTest = { SD_LEFT, NULL, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 2500, pstParam->enDir);	// 2500 * 1ms = 2.5sec

	pifTimer_Start(pstParam->pstTimerItem, 1000);	                        // 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_RIGHT ? SD_LEFT : SD_RIGHT;
}

void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 3);					// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_stSolenoidTest.pstSolenoid = pifSolenoid_Create(PIF_ID_AUTO, g_pstTimer1ms,
    		ST_2POINT, 30, actSolenoidOrder);										// 30ms
    if (!s_stSolenoidTest.pstSolenoid) return;
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return;

    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(g_pstTimer1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);					    	// 1000ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
