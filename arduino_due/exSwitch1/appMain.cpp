#include "appMain.h"


PifGpio g_gpio;
PifSensorSwitch g_push_switch;
PifSensorSwitch g_tilt_switch;


static void evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifGpio_WriteCell(&g_gpio, 0, state);
}

static void evtTiltSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	pifGpio_WriteCell(&g_gpio, 1, state);
}

BOOL appSetup()
{
    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, TM_PERIOD, 1000, TRUE)) return FALSE;	// 1ms
    pifSensor_AttachEvtChange(&g_push_switch.parent, evtPushSwitchChange, NULL);

	if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, TM_PERIOD, 1000, TRUE)) return FALSE;	// 1ms
	pifSensor_AttachEvtChange(&g_tilt_switch.parent, evtTiltSwitchChange, NULL);
	return TRUE;
}
