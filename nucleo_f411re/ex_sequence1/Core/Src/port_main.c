#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_log_tx;


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static uint32_t taskLedToggle(PifTask *p_task)
{
	static BOOL sw = OFF;

	(void)p_task;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sw);
	sw ^= 1;
	return 0;
}

static BOOL actLogStartTransfer(PifUart *p_uart)
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
		state = pifUart_EndGetTxData(&s_uart_log, s_log_tx);
		if (!(state & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_log_tx = 0;
			state = pifUart_StartGetTxData(&s_uart_log, &p_data, &s_log_tx);
			if (state & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, p_data, s_log_tx);
			}
		}
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) goto fail;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) goto fail;
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) goto fail;											// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;												// 256bytes

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;	// 500ms
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
