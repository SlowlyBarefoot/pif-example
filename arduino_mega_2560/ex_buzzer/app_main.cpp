#include "app_main.h"
#include "ex_buzzer.h"

#include "core/pif_log.h"
#include "sound/pif_buzzer.h"


PifTimerManager g_timer_1ms;
PifUart g_uart_log;

static PifBuzzer s_buzzer;

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
		pifBuzzer_Start(&s_buzzer, buzz[order]);
		order++;
		if (order >= 6) order = 0;
		state = 1;
		break;

	case 1:
		if (s_buzzer._state == BS_IDLE) {
			state = 0;
			delay = 500;
		}
		break;
	}
	return delay;
}

void appSetup()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;			// 1000us

	if (!pifUart_Init(&g_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
#ifdef USE_SERIAL
	g_uart_log.act_send_data = actLogSendData;
#endif
#ifdef USE_USART
	if (!pifUart_AllocTxBuffer(&g_uart_log, 64)) return;
	g_uart_log.act_start_transfer = actLogStartTransfer;
#endif

	if (!pifLog_AttachUart(&g_uart_log)) return;

    if (!pifBuzzer_Init(&s_buzzer, PIF_ID_AUTO, actBuzzerAction)) return;
    s_buzzer.evt_finish = _evtBuzzerFinish;

    if (!pifTaskManager_Add(TM_CHANGE_MS, 10, _taskBuzzer, NULL, TRUE)) return;		// 10ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
