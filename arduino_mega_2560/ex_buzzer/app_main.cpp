#include "app_main.h"


PifBuzzer g_buzzer;
PifTimerManager g_timer_1ms;

static const uint8_t buzz_0[] = {
    5, 5, 0xF2
};
static const uint8_t buzz_1[] = {
    35, 150, 0xF1
};
static const uint8_t buzz_2[] = {
    70, 200, 0xF0
};
static const uint8_t buzz_3[] = {
    10, 10, 10, 10, 10, 40, 40, 10, 40, 10, 40, 40, 10, 10, 10, 10, 10, 70, 0xF0
};
static const uint8_t buzz_4[] = {
    5, 5, 15, 5, 5, 5, 15, 30, 0xF0
};
static const uint8_t buzz_5[] = {
    4, 5, 4, 5, 8, 5, 15, 5, 8, 5, 4, 5, 4, 5, 0xF0
};
static const uint8_t* buzz[] = {
	buzz_0, buzz_1, buzz_2, buzz_3, buzz_4, buzz_5
};


static void _evtBuzzerFinish(PifId id)
{
	(void)id;

	pifLog_Printf(LT_INFO, "Buzzer Stop\n");
}

static uint16_t _taskBuzzer(PifTask *pstTask)
{
	static int order = 0, state = 0;
	uint16_t delay = 10;

	(void)pstTask;

	switch (state) {
	case 0:
		pifLog_Printf(LT_INFO, "Buzzer Start Order=%d", order);
		pifBuzzer_Start(&g_buzzer, buzz[order]);		order++;
		if (order >= 6) order = 0;
		state = 1;
		break;

	case 1:
		if (g_buzzer._state == BS_IDLE) {
			state = 0;
			delay = 500;
		}
		break;
	}
	return delay;
}

BOOL appSetup()
{
    g_buzzer.evt_finish = _evtBuzzerFinish;

	if (!pifTaskManager_Add(TM_CHANGE_MS, 10, _taskBuzzer, NULL, TRUE)) return FALSE;		// 10ms
	return TRUE;
}
