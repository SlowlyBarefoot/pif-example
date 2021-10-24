#include "appMain.h"
#include "exSwitch2.h"

#include "pif_led.h"
#include "pif_log.h"


PifSensor *g_pstPushSwitch = NULL;
PifSensor *g_pstTiltSwitch = NULL;
PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL = NULL;


void appSetup()
{
	PifComm *pstCommLog;
	PifPulseItem *pstTimerSwitch;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    pstTimerSwitch = pifPulse_AddItem(g_pstTimer1ms, PT_REPEAT);
    if (!pstTimerSwitch) return;
    pifPulse_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    g_pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
    if (!g_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstPushSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
    pifSensor_AttachEvtChange(g_pstPushSwitch, evtPushSwitchChange, NULL);

    g_pstTiltSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
	if (!g_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstTiltSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
	pifSensor_AttachEvtChange(g_pstTiltSwitch, evtTiltSwitchChange, NULL);

    pifPulse_StartItem(pstTimerSwitch, 20);											// 20ms
}
