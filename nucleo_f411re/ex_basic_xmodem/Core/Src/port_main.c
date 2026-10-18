#include "main.h"
#include "app_port.h"


static uint16_t s_log_tx;
static uint8_t s_log_rx;


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *p_data, state;

	s_log_tx = 0;
	state = pifUart_StartGetTxData(p_uart, &p_data, &s_log_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, p_data, s_log_tx);
		return TRUE;
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *p_data, state;

	if (huart->Instance == USART2) {
		state = pifUart_EndGetTxData(&g_uart, s_log_tx);
		if (!(state & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_log_tx = 0;
			state = pifUart_StartGetTxData(&g_uart, &p_data, &s_log_tx);
			if (state & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, p_data, s_log_tx);
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		pifUart_PutRxByte(&g_uart, s_log_rx);
		HAL_UART_Receive_IT(huart, &s_log_rx, 1);
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&g_uart, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&g_uart, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
	if (!pifUart_AttachRxTask(&g_uart, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) return FALSE;	// 200ms
	if (!pifUart_AllocRxBuffer(&g_uart, 64)) return FALSE;											// 64bytes
	if (!pifUart_AllocTxBuffer(&g_uart, 128)) return FALSE;											// 128bytes
	g_uart.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_log_rx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&g_uart, 256)) return FALSE;												// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) return FALSE;
	return TRUE;
}
