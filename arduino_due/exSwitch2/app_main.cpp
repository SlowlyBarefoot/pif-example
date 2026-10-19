#include "app_port.h"


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
    if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, OFF, NULL)) goto fail;
    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 1000, TRUE)) goto fail;	// 1ms
    pifSensor_AttachEvtChange(&g_push_switch.parent, evtPushSwitchChange, NULL);

	if (!pifSensorSwitch_Init(&g_tilt_switch, PIF_ID_AUTO, OFF, NULL)) goto fail;
	if (!pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, PIF_ID_AUTO, TM_PERIOD, 1000, TRUE)) goto fail;	// 1ms
	pifSensor_AttachEvtChange(&g_tilt_switch.parent, evtTiltSwitchChange, NULL);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) goto fail;														// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    pifTimer_Start(g_timer_switch, 20);												    						// 20ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***             exSwitch2            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
    pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
