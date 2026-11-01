#include "host.h"
#include "app_port.h"


static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	hostLed(sw);
	sw ^= 1;
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!g_timer_led) return FALSE;
	pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);
	return TRUE;
}
