#include "appMain.h"
#include "exSwitch3.h"

#include "pif_led.h"
#include "pif_log.h"


PifSensorSwitch g_push_switch;
PifSensorSwitch g_tilt_switch;
PifTimerManager g_timer_1ms;

static PifLed s_led;
static PifSensorSwitchFilter s_stPushSwitchFilter;
static PifSensorSwitchFilter s_stTiltSwitchFilter;


static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_PartChange(&s_led, 1 << 1, usLevel);
}

static void _evtTiltSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	pifLed_PartChange(&s_led, 1 << 2, usLevel);
}

void appSetup()
{
	static PifComm s_comm_log;
	PifTimer *pstTimerSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led, PIF_ID_AUTO, &g_timer_1ms, 3, actLedState)) return;
    if (!pifLed_AttachSBlink(&s_led, 500)) return;									// 500ms

    pstTimerSwitch = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimerSwitch) return;
    pifTimer_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);

    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, OFF)) return;
    if (!pifSensorSwitch_AttachTask(&g_push_switch, TM_RATIO, 3, TRUE)) return;		// 3%
    pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(&g_push_switch, PIF_SENSOR_SWITCH_FILTER_COUNT, 5, &s_stPushSwitchFilter)) return;

	if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, OFF)) return;
    if (!pifSensorSwitch_AttachTask(&g_tilt_switch, TM_RATIO, 3, TRUE)) return;		// 3%
	pifSensor_AttachEvtChange(&g_tilt_switch.parent, _evtTiltSwitchChange, NULL);
    if (!pifSensorSwitch_AttachFilter(&g_tilt_switch, PIF_SENSOR_SWITCH_FILTER_CONTINUE, 5, &s_stTiltSwitchFilter)) return;

    pifLed_SBlinkOn(&s_led, 1 << 0);

    pifTimer_Start(pstTimerSwitch, 20);											    // 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
