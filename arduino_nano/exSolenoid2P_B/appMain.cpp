#include "appMain.h"
#include "exSolenoid2P_B.h"

#include "pifLog.h"


#define PULSE_ITEM_COUNT    	5
#define SOLENOID_COUNT          1
#define TASK_COUNT              3


typedef struct {
	PIF_enSolenoidDir enDir;
    PIF_stSolenoid *pstSolenoid;
    PIF_stPulseItem *pstTimerItem;
} ST_SolenoidTest;


PIF_stPulse *g_pstTimer1ms = NULL;

static ST_SolenoidTest s_stSolenoidTest = { SD_enLeft, NULL, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 2500, pstParam->enDir);	// 2500 * 1ms = 2.5sec

	pifPulse_StartItem(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_enInfo, "_evtSolenoidFinish(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_enRight ? SD_enLeft : SD_enRight;
}

void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Init(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;							// 100%

    if (!pifSolenoid_Init(SOLENOID_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;							// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_stSolenoidTest.pstSolenoid = pifSolenoid_Add(PIF_ID_AUTO, ST_en2Point, 30, actSolenoidOrder);	// 30ms
    if (!s_stSolenoidTest.pstSolenoid) return;
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return;

    s_stSolenoidTest.pstTimerItem = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifPulse_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifPulse_StartItem(s_stSolenoidTest.pstTimerItem, 1000);										// 1000ms

    if (!pifTask_Add(TM_enPeriodMs, 500, taskLedToggle, NULL, TRUE)) return;						// 500ms
}
