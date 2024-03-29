#include "appMain.h"


PifLed g_led_l;
PifLed g_led_ry;
PifSensorSwitch g_push_switch;
PifSensorSwitch g_tilt_switch;
PifTimerManager g_timer_1ms;

PifTimer* g_timer_switch;


static void evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&g_led_ry, 1, state);
}

static void evtTiltSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifLed_PartChange(&g_led_ry, 2, state);
}

BOOL appSetup()
{
    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, OFF, NULL)) return FALSE;
    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_RATIO, 3, TRUE)) return FALSE;	// 3%
    g_push_switch.parent.evt_change = evtPushSwitchChange;

	if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, OFF, NULL)) return FALSE;
	if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, TM_RATIO, 3, TRUE)) return FALSE;	// 3%
	g_tilt_switch.parent.evt_change = evtTiltSwitchChange;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;										// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    pifTimer_Start(g_timer_switch, 20);												    		// 20ms
    return TRUE;
}
