#include "main.h"
#include "app_port.h"


static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	HAL_GPIO_WritePin(GPIOB, LD2_Pin, sw);
	sw ^= 1;
}

BOOL portSetup()
{
	pif_Init(NULL);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!g_timer_led) return FALSE;
	pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);
	return TRUE;
}
