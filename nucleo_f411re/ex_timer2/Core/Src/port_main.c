#include "main.h"
#include "app_port.h"


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static void evtLedRedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sw);
	sw ^= 1;
}

static void evtLedYellowToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, sw);
	sw ^= 1;
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 2)) goto fail;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

	if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, TIMER_100US_SIZE)) goto fail;	// 100us

	g_timer_red = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!g_timer_red) goto fail;
	pifTimer_AttachEvtFinish(g_timer_red, evtLedRedToggle, NULL, FALSE);

	g_timer_yellow = pifTimerManager_Add(&g_timer_100us, TT_REPEAT);
	if (!g_timer_yellow) goto fail;
	pifTimer_AttachEvtFinish(g_timer_yellow, evtLedYellowToggle, NULL, FALSE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
