#include "appMain.h"

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
#if USE_FILTER
	static PifNoiseFilterManager s_switch_filter;

    if (!pifNoiseFilterManager_Init(&s_switch_filter, 2)) return FALSE;
#endif

    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, OFF, NULL)) return FALSE;
	if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_PERIOD, 100000, TRUE)) return FALSE;	// 100ms
	pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, NULL);

    if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, OFF, NULL)) return FALSE;
    if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, TM_PERIOD, 100000, TRUE)) return FALSE;	// 100ms
    pifSensor_AttachEvtChange(&g_tilt_switch.parent, _evtTiltSwitchChange, NULL);

#if USE_FILTER
    g_push_switch.p_filter = pifNoiseFilterBit_AddCount(&s_switch_filter, 5);
    if (!g_push_switch.p_filter) return FALSE;
	g_tilt_switch.p_filter = pifNoiseFilterBit_AddContinue(&s_switch_filter, 5);
    if (!g_tilt_switch.p_filter) return FALSE;
#endif

    if (!pifLed_AttachSBlink(&g_led, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led, 1 << 0);

	if (!pifTimer_Start(g_timer_switch, 20)) return FALSE;											// 20ms
	return TRUE;
}
