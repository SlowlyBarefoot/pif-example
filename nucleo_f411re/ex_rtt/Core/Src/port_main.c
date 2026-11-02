#include "main.h"
#include "app_port.h"


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, unState & 1);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	// RTT channel 0, set up by RTT itself: the log and the command line. Read it with J-Link RTT Viewer, or
	// "rtt" of OpenOCD or pyOCD over the ST-LINK.
	if (!pifRtt_Init(&g_rtt, PIF_ID_AUTO, 0, 512)) return FALSE;										// 512 bytes
	if (!pifUart_AttachTxTask(&g_rtt.parent, PIF_ID_AUTO, TM_PERIOD, 1000, "RttTx")) return FALSE;		// 1ms
	if (!pifUart_AttachRxTask(&g_rtt.parent, PIF_ID_AUTO, TM_PERIOD, 20000, "RttRx")) return FALSE;	// 20ms

	pifLog_Init();
	if (!pifLog_AttachUart(&g_rtt.parent, 256)) return FALSE;											// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
	return TRUE;
}
