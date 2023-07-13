#include "appMain.h"
#include "exCollectSignal2.h"

#include "core/pif_collect_signal.h"
#include "core/pif_gpio.h"
#include "core/pif_log.h"
#include "core/pif_sequence.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


PifTimerManager g_timer_1ms;

static PifLed s_led_l;
static PifGpio s_gpio_rgb;
static PifLed s_led_collect;

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner);
static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner);

const PifSequencePhase s_astSequencePhaseList[] = {
		{ _fnSequenceStart, 1 },
		{ _fnSequenceRun, 2 },
		{ _fnSequenceStop, PIF_SEQUENCE_PHASE_NO_IDLE }
};

static BOOL bCollect = FALSE;

typedef struct {
	PifSensorSwitch stPushSwitch;
	PifSequence stSequence;
	BOOL bSequenceParam;
} TestStruct;

static TestStruct s_test[SEQUENCE_COUNT];


static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	TestStruct* p_test = (TestStruct*)p_issuer;

	(void)p_owner;
	(void)p_value;

	if (state) {
		if (p_test->stSequence._phase_no == PIF_SEQUENCE_PHASE_NO_IDLE) {
			pifSequence_Start(&p_test->stSequence);
		}
	}
	else {
		if (p_test->stSequence._phase_no != PIF_SEQUENCE_PHASE_NO_IDLE) {
			p_test->bSequenceParam = TRUE;
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
			pifLed_AllOn(&s_led_collect);
		    pifCollectSignal_Start();
			bCollect = TRUE;
		}
		else {
			pifLed_AllOff(&s_led_collect);
		    pifCollectSignal_Stop();
		    pifCollectSignal_PrintLog();
			bCollect = FALSE;
		}
	}
}

static PifSequenceResult _fnSequenceStart(PifSequence *pstOwner)
{
	uint8_t index;
	TestStruct* p_test = (TestStruct*)pstOwner->p_param;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(&s_gpio_rgb, index, ON);
		p_test->bSequenceParam = FALSE;
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceRun(PifSequence *pstOwner)
{
	TestStruct* p_test = (TestStruct*)pstOwner->p_param;

	if (p_test->bSequenceParam) {
		pstOwner->delay1us = 1000000UL;
		return SR_NEXT;
	}
	return SR_CONTINUE;
}

static PifSequenceResult _fnSequenceStop(PifSequence *pstOwner)
{
	uint8_t index;

	switch (pstOwner->step) {
	case PIF_SEQUENCE_STEP_INIT:
		index = pstOwner->_id - PIF_ID_SEQUENCE;
		pifGpio_WriteCell(&s_gpio_rgb, index, OFF);
		return SR_NEXT;

	default:
		// 어떤 오류가 발생하면 오류 처리후 SR_enFinish로 return할 것.
		// If any error occurs, return to SR_enFinish after processing the error.
		return SR_FINISH;
	}
	return SR_CONTINUE;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	PifSensorSwitch s_push_switch_collect;
	int i;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(8)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 4)) return;								// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;								// 1ms
    s_comm_log.act_receive_data = actLogReceiveData;
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

	if (!pifCollectSignal_InitHeap("example", 0x1000)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;													// 500ms

    if (!pifGpio_Init(&s_gpio_rgb, PIF_ID_AUTO, SEQUENCE_COUNT)) return;
    pifGpio_AttachActOut(&s_gpio_rgb, actGpioRGBState);
    pifGpioColSig_SetFlag(GP_CSF_ALL_BIT);

    if (!pifLed_Init(&s_led_collect, PIF_ID_AUTO, &g_timer_1ms, 1, actLedCollectState)) return;

    for (i = 0; i < SEQUENCE_COUNT; i++) {
		if (!pifSensorSwitch_Init(&s_test[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire, &s_test[i])) return;
	    if (!pifSensorSwitch_AttachTaskAcquire(&s_test[i].stPushSwitch, TM_PERIOD_MS, 5, TRUE)) return;	// 5ms
	    pifSensorSwitch_SetCsFlag(&s_test[i].stPushSwitch, SS_CSF_FILTER_BIT);
		pifSensor_AttachEvtChange(&s_test[i].stPushSwitch.parent, _evtPushSwitchChange);

	    if (!pifSequence_Init(&s_test[i].stSequence, PIF_ID_SEQUENCE + i, &g_timer_1ms, 10,				// 10ms
				s_astSequencePhaseList,	&s_test[i])) return;
    }
    pifSequenceColSig_SetFlag(SQ_CSF_ALL_BIT);

	if (!pifSensorSwitch_Init(&s_push_switch_collect, PIF_ID_AUTO, 0, actPushSwitchCollectAcquire, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&s_push_switch_collect, TM_PERIOD_MS, 5, TRUE)) return;		// 5ms
	pifSensor_AttachEvtChange(&s_push_switch_collect.parent, _evtPushSwitchCollectChange);

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
