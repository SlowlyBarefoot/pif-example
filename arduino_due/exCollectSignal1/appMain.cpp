#include "appMain.h"


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
	pifGpio_WriteCell(&g_gpio_rgb, p_test - g_test, ON);
	pifSequence_Delay(p_owner, _fnSequenceStop, 100);	// 100ms
}

static void _fnSequenceStop(PifSequence *p_owner)
{
	TestStruct *p_test = (TestStruct*)p_owner->p_param;

	pifCollectSignal_Put(&p_test->cs_step, 2);
	pifGpio_WriteCell(&g_gpio_rgb, p_test - g_test, OFF);
}

BOOL appSetup()
{
	static PifNoiseFilterManager s_switch_filter;
	int i;

    if (!pifNoiseFilterManager_Init(&s_switch_filter, SEQUENCE_COUNT + 1)) return FALSE;

    if (!pifGpio_SetCsFlag(&g_gpio_rgb, GP_CSF_ALL_BIT)) return FALSE;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_test[i].push_switch, PIF_ID_AUTO, TM_PERIOD, 5000, TRUE)) return FALSE;	// 5ms
	    g_test[i].push_switch.p_filter = pifNoiseFilterBit_AddCount(&s_switch_filter, 7);								// 35ms
	    if (!g_test[i].push_switch.p_filter) return FALSE;
	    if (!pifSensorSwitch_SetCsFlag(&g_test[i].push_switch, SS_CSF_RAW_BIT)) return FALSE;
	    pifSensor_AttachEvtChange(&g_test[i].push_switch.parent, _evtPushSwitchChange, &g_test[i]);

	    if (!pifSequence_Init(&g_test[i].sequence, PIF_ID_SEQUENCE + i, &g_test[i])) return FALSE;
	    if (!pifCollectSignal_AddChannel(&g_test[i].cs_step, "SQ", PIF_ID_SEQUENCE + i, CSVT_REG, 2, 0)) return FALSE;
    }

    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch_collect, PIF_ID_AUTO, TM_PERIOD, 5000, TRUE)) return FALSE;		// 5ms
    g_push_switch_collect.p_filter = pifNoiseFilterBit_AddCount(&s_switch_filter, 7);									// 35ms
    if (!g_push_switch_collect.p_filter) return FALSE;
    pifSensor_AttachEvtChange(&g_push_switch_collect.parent, _evtPushSwitchCollectChange, NULL);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;																	// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);
	return TRUE;
}
