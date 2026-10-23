#include "app_port.h"


PifGpio g_gpio_rgb;
PifLed g_led_collect;
PifLed g_led_l;
PifSensorSwitch g_push_switch_collect;
PifTimerManager g_timer_1ms;

TestStruct g_test[SEQUENCE_COUNT];

static void _fnSequenceStart(PifSequence *p_owner);
static void _fnSequenceStop(PifSequence *p_owner);


static void _evtPushSwitchChange(PifSensor *p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	TestStruct *p_test = (TestStruct*)p_issuer;

	(void)p_owner;
	(void)p_value;

	if (state) {
		pifSequence_Start(&p_test->sequence, _fnSequenceStart);
	}
}

static void _evtPushSwitchCollectChange(PifSensor *p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
		if (!pifCollectSignal_IsCollecting()) {		// Fails while the last capture is still printing
			if (pifCollectSignal_Start()) {
				pifLed_AllOn(&g_led_collect);
			}
		}
		else {
			pifLed_AllOff(&g_led_collect);
		    pifCollectSignal_Stop();
		}
	}
}

static void _fnSequenceStart(PifSequence *p_owner)
{
	TestStruct *p_test = (TestStruct*)p_owner->p_param;

	pifCollectSignal_Put(&p_test->cs_step, 1);
	pifGpio_WriteCell(&g_gpio_rgb, p_test - g_test, TRUE);
	pifSequence_Delay(p_owner, _fnSequenceStop, 100);	// 100ms
}

static void _fnSequenceStop(PifSequence *p_owner)
{
	TestStruct *p_test = (TestStruct*)p_owner->p_param;

	pifCollectSignal_Put(&p_test->cs_step, 2);
	pifGpio_WriteCell(&g_gpio_rgb, p_test - g_test, FALSE);
}

BOOL appSetup()
{
	PifTask* p_task;
	static PifNfBit s_switch_filter[SEQUENCE_COUNT + 1];
	int i;

    if (!pifGpio_SetCsFlag(&g_gpio_rgb, GP_CSF_ALL_BIT)) goto fail;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    p_task = pifSensorSwitch_AttachTaskAcquire(&g_test[i].push_switch, PIF_ID_AUTO, TM_PERIOD, 5000, TRUE);	// 5ms
	    if (!p_task) goto fail;
	    p_task->name = "PushSw";
	    if (!pifNoiseFilterBit_InitCount(&s_switch_filter[i], 7)) goto fail;
	    if (!pifSensorSwitch_AttachFilter(&g_test[i].push_switch, &s_switch_filter[i].parent)) goto fail;
	    if (!pifSensorSwitch_SetCsFlag(&g_test[i].push_switch, SS_CSF_RAW_BIT)) goto fail;
	    pifSensor_AttachEvtChange(&g_test[i].push_switch.parent, _evtPushSwitchChange, &g_test[i]);

	    if (!pifSequence_Init(&g_test[i].sequence, PIF_ID_SEQUENCE + i, &g_test[i])) goto fail;
	    g_test[i].sequence._p_task->name = "Sequence";
	    if (!pifCollectSignal_AddChannel(&g_test[i].cs_step, "SQ", PIF_ID_SEQUENCE + i, CSVT_REG, 2, 0)) goto fail;
    }

    p_task = pifSensorSwitch_AttachTaskAcquire(&g_push_switch_collect, PIF_ID_AUTO, TM_PERIOD, 5000, TRUE);		// 5ms
    if (!p_task) goto fail;
    p_task->name = "CollectSw";
    if (!pifNoiseFilterBit_InitCount(&s_switch_filter[SEQUENCE_COUNT], 7)) goto fail;
    if (!pifSensorSwitch_AttachFilter(&g_push_switch_collect, &s_switch_filter[SEQUENCE_COUNT].parent)) goto fail;
    pifSensor_AttachEvtChange(&g_push_switch_collect.parent, _evtPushSwitchCollectChange, NULL);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) goto fail;																	// 500ms
    g_led_l._p_task->name = "LedL";
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exCollectSignal1         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
