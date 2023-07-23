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
	static PifNoiseFilterBit s_push_switch_filter;
	static PifNoiseFilterBit s_tilt_switch_filter;
#endif

#if USE_FILTER
    if (!pifNoiseFilterBit_Init(&s_push_switch_filter, 5)) return FALSE;
    if (!pifNoiseFilterBit_Init(&s_tilt_switch_filter, 5)) return FALSE;
    if (!pifNoiseFilterBit_SetContinue(&s_tilt_switch_filter, 5)) return FALSE;
#endif

	if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_PERIOD_MS, 100, TRUE)) return FALSE;	// 100ms
    g_push_switch.parent.evt_change = _evtPushSwitchChange;

    if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, TM_PERIOD_MS, 100, TRUE)) return FALSE;	// 100ms
	g_tilt_switch.parent.evt_change = _evtTiltSwitchChange;

#if USE_FILTER
    g_push_switch.p_filter = &s_push_switch_filter.parent;
	g_tilt_switch.p_filter = &s_tilt_switch_filter.parent;
#endif

    if (!pifLed_AttachSBlink(&g_led, 500)) return FALSE;											// 500ms
    pifLed_SBlinkOn(&g_led, 1 << 0);

	if (!pifTimer_Start(g_timer_switch, 20)) return FALSE;											// 20ms
	return TRUE;
}
