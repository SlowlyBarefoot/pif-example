#include "appMain.h"
#include "exSwitch3.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "filter/pif_noise_filter_bit.h"


#define USE_FILTER		1


PifSensorSwitch g_push_switch;
PifSensorSwitch g_tilt_switch;
PifTimerManager g_timer_1ms;

static PifLed s_led;


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&s_led, 1 << 1, state);
}

static void _evtTiltSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&s_led, 1 << 2, state);
}

void appSetup()
{
	static PifComm s_comm_log;
#if USE_FILTER
	static PifNoiseFilterBit s_push_switch_filter;
	static PifNoiseFilterBit s_tilt_switch_filter;
#endif
	PifTimer *pstTimerSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;						// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;						// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led, PIF_ID_AUTO, &g_timer_1ms, 3, actLedState)) return;
    if (!pifLed_AttachSBlink(&s_led, 500)) return;												// 500ms

#if USE_FILTER
    if (!pifNoiseFilterBit_Init(&s_push_switch_filter, 5)) return;
    if (!pifNoiseFilterBit_Init(&s_tilt_switch_filter, 5)) return;
    if (!pifNoiseFilterBit_SetContinue(&s_tilt_switch_filter, 5)) return;
#endif

    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, OFF, NULL, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_PERIOD_MS, 100, TRUE)) return;	// 100ms
    pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange);

    if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, OFF, NULL, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, TM_PERIOD_MS, 100, TRUE)) return;	// 100ms
	pifSensor_AttachEvtChange(&g_tilt_switch.parent, _evtTiltSwitchChange);

#if USE_FILTER
    g_push_switch.p_filter = &s_push_switch_filter.parent;
	g_tilt_switch.p_filter = &s_tilt_switch_filter.parent;
#endif

    pifLed_SBlinkOn(&s_led, 1 << 0);

    pstTimerSwitch = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!pstTimerSwitch) return;
    pifTimer_AttachEvtFinish(pstTimerSwitch, evtSwitchAcquire, NULL);
    if (!pifTimer_Start(pstTimerSwitch, 20)) return;											// 20ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
