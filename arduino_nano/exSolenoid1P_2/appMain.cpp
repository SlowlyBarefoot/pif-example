#include "appMain.h"
#include "exSolenoid1P_2.h"

#include "pifLog.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SOLENOID_COUNT          1
#define TASK_COUNT              3


typedef struct {
	SWITCH nSwitch;
    PIF_stSolenoid *pstSolenoid;
    PIF_stPulseItem *pstTimerItem;
} ST_SolenoidTest;


PIF_stPulse *g_pstTimer1ms = NULL;

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

	pifLog_Printf(LT_enInfo, "_evtSolenoidFinish(%d)", pstParam->nSwitch);

	pstParam->nSwitch ^= 1;
}

void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_AddRatio(100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    if (!pifSolenoid_Init(SOLENOID_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, pstCommLog, TRUE)) return;	// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_stSolenoidTest.pstSolenoid = pifSolenoid_Add(PIF_ID_AUTO, ST_en1Point, 0, actSolenoidOrder);
    if (!s_stSolenoidTest.pstSolenoid) return;

    s_stSolenoidTest.pstTimerItem = pifPulse_AddItem(g_pstTimer1ms, PT_enOnce);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifPulse_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifPulse_StartItem(s_stSolenoidTest.pstTimerItem, 1000);				// 1000ms

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL, TRUE)) return;		// 500ms
}
