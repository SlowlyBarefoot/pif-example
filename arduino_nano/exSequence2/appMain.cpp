#include "appMain.h"


PifLed g_led_l;
PifLed g_led_rgb;
PifTimerManager g_timer_1ms;

SequenceTest g_stSequenceTest[SEQUENCE_COUNT];

static void _fnSequenceStart(PifSequence *p_owner);
static void _fnSequenceStop(PifSequence *p_owner);


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	SequenceTest* p_test = (SequenceTest*)p_issuer;

	(void)p_value;

	if (state) {
		pifSequence_Start(&p_test->stSequence, _fnSequenceStart);
	}
	pifLog_Printf(LT_INFO, "Switch(%d): %d", p_owner->_id, state);
}

static void _fnSequenceStart(PifSequence *p_owner)
{
	uint8_t index;

	index = p_owner->_id - PIF_ID_SEQUENCE;
	pifLed_PartOn(&g_led_rgb, 1 << index);
	pifSequence_NextDelay(p_owner, _fnSequenceStop, 1000);		// 1000ms
	pifLog_Printf(LT_INFO, "Sequence(%x): Start", p_owner->_id);
}

static void _fnSequenceStop(PifSequence *p_owner)
{
	uint8_t index;

	index = p_owner->_id - PIF_ID_SEQUENCE;
	pifLed_PartOff(&g_led_rgb, 1 << index);
	pifLog_Printf(LT_INFO, "Sequence(%x): Stop", p_owner->_id);
}

static void _evtSequenceError(PifSequence *p_owner)
{
	(void)p_owner;

	pifLog_Printf(LT_ERROR, "Sequence Error: %d", pif_error);
}

BOOL appSetup()
{
	int i;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
	    if (!pifSensorSwitch_AttachTaskAcquire(&g_stSequenceTest[i].stPushSwitch, TM_PERIOD, 10000, TRUE)) return FALSE;	// 10ms
		pifSensor_AttachEvtChange(&g_stSequenceTest[i].stPushSwitch.parent, _evtPushSwitchChange, &g_stSequenceTest[i]);

	    if (!pifSequence_Init(&g_stSequenceTest[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, &g_stSequenceTest[i])) return FALSE;
	    g_stSequenceTest[i].stSequence.evt_error = _evtSequenceError;
    }

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;																	// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exSequence2           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
