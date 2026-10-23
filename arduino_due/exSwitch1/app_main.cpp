#include "app_port.h"


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
	PifTask* p_task;

    p_task = pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 1000, TRUE);	// 1ms
    if (!p_task) goto fail;
    p_task->name = "PushSw";
    pifSensor_AttachEvtChange(&g_push_switch.parent, evtPushSwitchChange, NULL);

	p_task = pifSensorSwitch_AttachTaskAcquire(&g_tilt_switch, PIF_ID_AUTO, TM_PERIOD, 1000, TRUE);	// 1ms
	if (!p_task) goto fail;
	p_task->name = "TiltSw";
	pifSensor_AttachEvtChange(&g_tilt_switch.parent, evtTiltSwitchChange, NULL);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***             exSwitch1            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
