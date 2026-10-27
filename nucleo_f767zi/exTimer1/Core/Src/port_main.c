#include "main.h"
#include "app_port.h"


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	HAL_GPIO_WritePin(GPIOB, LD2_Pin, sw);
	sw ^= 1;
}

BOOL portSetup()
{
	HAL_TIM_Base_Start(&htim2);		// 1 MHz free running counter for pif_Init()

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!g_timer_led) return FALSE;
	pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);
	return TRUE;
}
