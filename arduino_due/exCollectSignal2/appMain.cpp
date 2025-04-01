#include "appMain.h"


PifLed g_led_l;
PifGpio g_gpio_rgb;
PifLed g_led_collect;
PifSensorSwitch g_push_switch_collect;
PifTimerManager g_timer_1ms;

TestStruct g_test[SEQUENCE_COUNT];

static void _fnSequenceStart(PifSequence* p_owner);
static void _fnSequenceStop(PifSequence* p_owner);

static BOOL bCollect = FALSE;


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	TestStruct* p_test = (TestStruct*)p_issuer;

	(void)p_owner;
	(void)p_value;

	if (state) {
		if (pifSequence_IsRunning(&p_test->stSequence)) {
			pifSequence_Start(&p_test->stSequence, _fnSequenceStart);
		}
	}
}

static void _evtPushSwitchCollectChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
		if (!bCollect) {
			pifLed_AllOn(&g_led_collect);
		    pifCollectSignal_Start();
			bCollect = TRUE;
		}
		else {
			pifLed_AllOff(&g_led_collect);
		    pifCollectSignal_Stop();
		    pifCollectSignal_PrintLog();
			bCollect = FALSE;
		}
	}
}

static void _fnSequenceStart(PifSequence* p_owner)
{
	uint8_t index;

	index = p_owner->_id - PIF_ID_SEQUENCE;
	pifGpio_WriteCell(&g_gpio_rgb, index, ON);
	pifSequence_NextDelay(p_owner, _fnSequenceStop, 100);	// 100ms
}

static void _fnSequenceStop(PifSequence* p_owner)
{
	uint8_t index;

	index = p_owner->_id - PIF_ID_SEQUENCE;
	pifGpio_WriteCell(&g_gpio_rgb, index, OFF);
}

BOOL appSetup()
{
	int i;

    pifGpioColSig_SetFlag(GP_CSF_ALL_BIT);

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_test[i].stPushSwitch, TM_PERIOD, 5000, TRUE)) return FALSE;	// 5ms
	    pifSensorSwitch_SetCsFlag(&g_test[i].stPushSwitch, SS_CSF_FILTER_BIT);
	    pifSensor_AttachEvtChange(&g_test[i].stPushSwitch.parent, _evtPushSwitchChange, &g_test[i]);

	    if (!pifSequence_Init(&g_test[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, &g_test[i])) return FALSE;
    }

    pifSequenceColSig_SetFlag(SQ_CSF_ALL_BIT);

    if (!pifSensorSwitch_AttachTaskAcquire(&g_push_switch_collect, TM_PERIOD, 5000, TRUE)) return FALSE;		// 5ms
    pifSensor_AttachEvtChange(&g_push_switch_collect.parent, _evtPushSwitchCollectChange, NULL);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;														// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
