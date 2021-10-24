#include "appMain.h"
#include "exSolenoid2P_B.h"

#include "pif_log.h"


typedef struct {
	PifSolenoidDir enDir;
    PifSolenoid *pstSolenoid;
    PifPulseItem *pstTimerItem;
} ST_SolenoidTest;


PifPulse *g_pstTimer1ms = NULL;

static ST_SolenoidTest s_stSolenoidTest = { SD_LEFT, NULL, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 2500, pstParam->enDir);	// 2500 * 1ms = 2.5sec

	pifPulse_StartItem(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_RIGHT ? SD_LEFT : SD_RIGHT;
}

void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_stSolenoidTest.pstSolenoid = pifSolenoid_Create(PIF_ID_AUTO, g_pstTimer1ms,
    		ST_2POINT, 30, actSolenoidOrder);										// 30ms
    if (!s_stSolenoidTest.pstSolenoid) return;
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return;

    s_stSolenoidTest.pstTimerItem = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifPulse_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifPulse_StartItem(s_stSolenoidTest.pstTimerItem, 1000);						// 1000ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
}
