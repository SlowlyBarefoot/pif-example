#include "app_port.h"

#include "filter/pif_noise_filter_bit.h"


#define USE_FILTER		1


PifLed g_led;
PifSensorSwitch g_push_switch;
PifSensorSwitch g_tilt_switch;
PifTimerManager g_timer_1ms;

PifTimer* g_timer_switch;


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&g_led, 1 << 1, state);
}

static void _evtTiltSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&g_led, 1 << 2, state);
}

BOOL appSetup()
{
	PifTask* p_task;
#if USE_FILTER
	static PifNfBit s_switch_filter[2];
#endif

    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, FALSE, NULL)) return FALSE;
	p_task = pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 100000, TRUE);	// 100ms
	if (!p_task) return FALSE;
	p_task->name = "PushSw";
	pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, NULL);

    if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, FALSE, NULL)) return FALSE;
    p_task = pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, PIF_ID_AUTO, TM_PERIOD, 100000, TRUE);	// 100ms
    if (!p_task) return FALSE;
    p_task->name = "TiltSw";
    pifSensor_AttachEvtChange(&g_tilt_switch.parent, _evtTiltSwitchChange, NULL);

#if USE_FILTER
    if (!pifNoiseFilterBit_InitCount(&s_switch_filter[0], 5)) return FALSE;
    if (!pifSensorSwitch_AttachFilter(&g_push_switch, &s_switch_filter[0].parent)) return FALSE;
	if (!pifNoiseFilterBit_InitContinue(&s_switch_filter[1], 5)) return FALSE;
	if (!pifSensorSwitch_AttachFilter(&g_tilt_switch, &s_switch_filter[1].parent)) return FALSE;
#endif

    if (!pifLed_AttachSBlink(&g_led, 500)) return FALSE;														// 500ms
    g_led._p_task->name = "Led";
    pifLed_SBlinkOn(&g_led, 1 << 0);

	if (!pifTimer_Start(g_timer_switch, 20)) return FALSE;														// 20ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exSwitch3             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
    pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;
}
