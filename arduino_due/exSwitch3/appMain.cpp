#include "appMain.h"
#include "exSwitch3.h"

#include "pif_led.h"
#include "pif_log.h"


PifSensor *g_pstPushSwitch = NULL;
PifSensor *g_pstTiltSwitch = NULL;
PifTimerManager *g_pstTimer1ms = NULL;

static PifLed *s_pstLed = NULL;
static PifSensorSwitchFilter s_stPushSwitchFilter;
static PifSensorSwitchFilter s_stTiltSwitchFilter;


static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_EachChange(s_pstLed, 1, usLevel);
}

static void _evtTiltSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_EachChange(s_pstLed, 2, usLevel);
}

void appSetup()
{
	PifComm *pstCommLog;
	PifTimer *pstTimerSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 2);					// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLed = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 3, actLedState);
    if (!s_pstLed) return;
    if (!pifLed_AttachBlink(s_pstLed, 500)) return;									// 500ms

    pstTimerSwitch = pifTimerManager_Add(g_pstTimer1ms, TT_REPEAT);
    if (!pstTimerSwitch) return;
    pifTimer_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    g_pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
    if (!g_pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstPushSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
    pifSensor_AttachEvtChange(g_pstPushSwitch, _evtPushSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(g_pstPushSwitch, PIF_SENSOR_SWITCH_FILTER_COUNT, 5, &s_stPushSwitchFilter)) return;

    g_pstTiltSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, OFF);
	if (!g_pstTiltSwitch) return;
    if (!pifSensorSwitch_AttachTask(g_pstTiltSwitch, TM_RATIO, 3, TRUE)) return;	// 3%
	pifSensor_AttachEvtChange(g_pstTiltSwitch, _evtTiltSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(g_pstTiltSwitch, PIF_SENSOR_SWITCH_FILTER_CONTINUE, 5, &s_stTiltSwitchFilter)) return;

    pifLed_BlinkOn(s_pstLed, 0);

    pifTimer_Start(pstTimerSwitch, 20);											    // 20ms

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
