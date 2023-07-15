#include "appMain.h"
#include "ex_hc_sr04.h"

#include "core/pif_log.h"


PifHcSr04 g_hcsr04;
PifTimerManager g_timer_1ms;


static void _evtHcSr04Distance(int32_t distance)
{
	pifLog_Printf(LT_INFO, "Distance = %dcm", distance);
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifUart s_uart_log;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

    if (!pifHcSr04_Init(&g_hcsr04, PIF_ID_AUTO)) return;
	g_hcsr04.act_trigger = actHcSr04Trigger;
	g_hcsr04.evt_read = _evtHcSr04Distance;
	if (!pifHcSr04_StartTrigger(&g_hcsr04, 70)) return;								// 70ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
