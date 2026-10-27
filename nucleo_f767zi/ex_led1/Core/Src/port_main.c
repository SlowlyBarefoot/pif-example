#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *pucData, ucState;

	s_usLogTx = 0;
	ucState = pifUart_StartGetTxData(p_uart, &pucData, &s_usLogTx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart3, pucData, s_usLogTx);
		return TRUE;
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *pucData, ucState;

  if (huart->Instance == USART3) {
		ucState = pifUart_EndGetTxData(&s_uart_log, s_usLogTx);
		if (!(ucState & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_usLogTx = 0;
			ucState = pifUart_StartGetTxData(&s_uart_log, &pucData, &s_usLogTx);
			if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, pucData, s_usLogTx);
			}
		}
	}
}

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	HAL_GPIO_WritePin(GPIOB, LD2_Pin, state & 1);
}

static void actLedRGBState(PifId id, uint32_t state)
{
	(void)id;

	HAL_GPIO_WritePin(GPIOD, LDRed_Pin, state & 1);
	HAL_GPIO_WritePin(GPIOD, LDGreen_Pin, (state >> 1) & 1);
	HAL_GPIO_WritePin(GPIOD, LDBlue_Pin, (state >> 2) & 1);
	pifLog_Printf(LT_INFO, "RGB:%u S:%x", __LINE__, state);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart3.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AllocTxBuffer(&s_uart_log, 64)) return FALSE;
	s_uart_log.act_start_transfer = actLogStartTransfer;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;										// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

	if (!pifLed_Init(&g_led_rgb, PIF_ID_AUTO, 3, actLedRGBState)) return FALSE;
	return TRUE;
}
