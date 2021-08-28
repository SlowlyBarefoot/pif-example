#include "appMain.h"
#include "exSwitch2.h"

#include "pifLed.h"
#include "pifLog.h"


#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define SWITCH_COUNT            2
#define TASK_COUNT              3


PIF_stSensor *g_pstPushSwitch = NULL;
PIF_stSensor *g_pstTiltSwitch = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;


void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stPulseItem *pstTimerSwitch;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    pstTimerSwitch = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimerSwitch) return;
    pifPulse_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    g_pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
    if (!g_pstPushSwitch) return;
    pifSensor_AttachEvtChange(g_pstPushSwitch, evtPushSwitchChange, NULL);

    g_pstTiltSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, OFF);
	if (!g_pstTiltSwitch) return;
	pifSensor_AttachEvtChange(g_pstTiltSwitch, evtTiltSwitchChange, NULL);

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms
    if (!pifTask_AddRatio(3, pifSensorSwitch_taskAll, NULL)) return;		// 3%

    pifPulse_StartItem(pstTimerSwitch, 20);									// 20ms
}
