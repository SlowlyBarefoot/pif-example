#include "appMain.h"
#include "exSwitch3.h"

#include "pifLed.h"
#include "pifLog.h"


PIF_stSensor *g_pstPushSwitch = NULL;
PIF_stSensor *g_pstTiltSwitch = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLed = NULL;
static PIF_stSensorSwitchFilter s_stPushSwitchFilter;
static PIF_stSensorSwitchFilter s_stTiltSwitchFilter;


static void _evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_EachChange(s_pstLed, 1, usLevel);
}

static void _evtTiltSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_EachChange(s_pstLed, 2, usLevel);
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stPulseItem *pstTimerSwitch;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;			// 100%

    pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_enPeriodMs, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLed = pifLed_Init(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedState);
    if (!s_pstLed) return;
    if (!pifLed_AttachBlink(s_pstLed, 500)) return;									// 500ms
    pifLed_BlinkOn(s_pstLed, 0);

    pstTimerSwitch = pifPulse_AddItem(g_pstTimer1ms, PT_enRepeat);
    if (!pstTimerSwitch) return;
    pifPulse_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    g_pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
    if (!g_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstPushSwitch, TM_enRatio, 3, TRUE)) return;	// 3%
    pifSensor_AttachEvtChange(g_pstPushSwitch, _evtPushSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(g_pstPushSwitch, PIF_SENSOR_SWITCH_FILTER_COUNT, 5, &s_stPushSwitchFilter)) return;

    g_pstTiltSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
	if (!g_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstTiltSwitch, TM_enRatio, 3, TRUE)) return;	// 3%
	pifSensor_AttachEvtChange(g_pstTiltSwitch, _evtTiltSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(g_pstTiltSwitch, PIF_SENSOR_SWITCH_FILTER_CONTINUE, 5, &s_stTiltSwitchFilter)) return;

    pifPulse_StartItem(pstTimerSwitch, 20);											// 20ms
}
