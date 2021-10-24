#include "appMain.h"
#include "exSolenoid1P_2.h"

#include "pif_log.h"


typedef struct {
	SWITCH nSwitch;
    PifSolenoid *pstSolenoid;
    PifPulseItem *pstTimerItem;
} ST_SolenoidTest;


PifPulse *g_pstTimer1ms = NULL;

static ST_SolenoidTest s_stSolenoidTest = {	OFF, NULL, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

	if (pstParam->nSwitch) {
		pifSolenoid_ActionOn(pstParam->pstSolenoid, 0);
	}
	else {
		pifSolenoid_ActionOff(pstParam->pstSolenoid);
	}

	pifPulse_StartItem(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->nSwitch);

	pstParam->nSwitch ^= 1;
}

void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;			// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_stSolenoidTest.pstSolenoid = pifSolenoid_Create(PIF_ID_AUTO, g_pstTimer1ms,
    		ST_1POINT, 0, actSolenoidOrder);
    if (!s_stSolenoidTest.pstSolenoid) return;

    s_stSolenoidTest.pstTimerItem = pifPulse_AddItem(g_pstTimer1ms, PT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifPulse_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifPulse_StartItem(s_stSolenoidTest.pstTimerItem, 1000);						// 1000ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
}
