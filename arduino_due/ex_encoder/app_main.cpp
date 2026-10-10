#include "app_main.h"


#define ENCODER_RESOLUTION		ENCODER_RES_X4
#define ENCODER_TIMEOUT_US		500000UL		// The speed reads as stopped after 0.5s without a step

#define LOG_PERIOD_US			500000UL		// 500ms


PifEncoder g_encoder;
PifTimerManager g_timer_1ms;

#if USE_GENERATOR

// A leading B, so that the encoder counts up.
static const uint8_t kPhase[4] = { 0, ENCODER_PHASE_A, ENCODER_PHASE_A | ENCODER_PHASE_B, ENCODER_PHASE_B };

// Each stage steps every `interval` ms in `direction` for `duration` ms.
static const struct {
	int8_t direction;
	uint16_t interval;
	uint16_t duration;
} kProfile[] = {
	{  0,  0, 1000 },		// Stop, also while setup() attaches the interrupts
	{  1,  1, 3000 },		// 1000 quarter steps/s
	{  1,  2, 3000 },		// 500
	{  1, 10, 3000 },		// 100
	{  0,  0, 2000 },		// Stop
	{ -1,  4, 3000 },		// -250
	{ -1,  1, 3000 },		// -1000
	{  0,  0, 2000 }		// Stop
};

static AppActGenerate s_act_generate;
static volatile int32_t s_gen_raw;		// Quarter steps generated
static uint8_t s_gen_phase;
static uint8_t s_stage;
static uint16_t s_stage_ms;
static uint16_t s_interval_ms;


/**
 * @brief Generates the quadrature signal from the 1ms timer interrupt, so that its timing is exact.
 */
static void _evtGenerate(PifIssuerP p_issuer)
{
	(void)p_issuer;

	if (++s_stage_ms >= kProfile[s_stage].duration) {
		s_stage_ms = 0;
		s_interval_ms = 0;
		s_stage = (s_stage + 1) % (sizeof(kProfile) / sizeof(kProfile[0]));
	}
	if (!kProfile[s_stage].direction) return;

	if (++s_interval_ms >= kProfile[s_stage].interval) {
		s_interval_ms = 0;
		s_gen_phase = (s_gen_phase + kProfile[s_stage].direction) & 3;
		s_gen_raw += kProfile[s_stage].direction;
		(*s_act_generate)(kPhase[s_gen_phase]);
	}
}

#endif	// USE_GENERATOR

static const char* _resultName(PifEncoderResult result)
{
	switch (result) {
	case ENCODER_R_OK:		return "OK";
	case ENCODER_R_NO_DATA:	return "NO_DATA";
	case ENCODER_R_TIMEOUT:	return "TIMEOUT";
	}
	return "?";
}

static uint32_t _taskLog(PifTask* p_task)
{
	PifEncoderResult result;
	float speed;

	(void)p_task;

	result = pifEncoder_ReadSpeed(&g_encoder, &speed);

#if USE_GENERATOR
	int32_t expect_speed = 0;
	int32_t gen_raw = s_gen_raw;
	uint8_t stage = s_stage;
	uint8_t div = 4 >> ENCODER_RESOLUTION;

	if (kProfile[stage].direction) {
		expect_speed = kProfile[stage].direction * 1000L / kProfile[stage].interval / div;
	}
	// Rounded down like the encoder, so that Pos follows Gen in every resolution. Err must stay 0.
	int32_t gen_pos = gen_raw >= 0 ? gen_raw / div : -((-gen_raw - 1) / div) - 1;
	pifLog_Printf(LT_INFO, "S%u Gen:%ld Pos:%ld Dir:%d Spd:%1f(%ld) %s Err:%lu",
			stage, gen_pos, pifEncoder_GetPosition(&g_encoder), pifEncoder_GetDirection(&g_encoder),
			(double)speed, expect_speed, _resultName(result), pifEncoder_GetErrorCount(&g_encoder));
#else
	pifLog_Printf(LT_INFO, "Pos:%ld Dir:%d Spd:%1f %s Err:%lu",
			pifEncoder_GetPosition(&g_encoder), pifEncoder_GetDirection(&g_encoder),
			(double)speed, _resultName(result), pifEncoder_GetErrorCount(&g_encoder));
#endif
    return 0;
}

BOOL appSetup(AppActGenerate act_generate)
{
	if (!pifEncoder_Init(&g_encoder, PIF_ID_AUTO, ENCODER_RESOLUTION)) return FALSE;
	pifEncoder_SetTimeout(&g_encoder, ENCODER_TIMEOUT_US);

#if USE_GENERATOR
	PifTimer* p_timer;

	s_act_generate = act_generate;
	(*s_act_generate)(kPhase[0]);

	p_timer = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!p_timer) return FALSE;
	pifTimer_AttachEvtIntFinish(p_timer, _evtGenerate, NULL);
	if (!pifTimer_Start(p_timer, 1)) return FALSE;													// 1ms
#else
	(void)act_generate;
#endif

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, LOG_PERIOD_US, _taskLog, NULL, TRUE)) return FALSE;
	return TRUE;
}
