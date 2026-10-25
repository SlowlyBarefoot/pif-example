#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;
static uint8_t s_ucLogRx;


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *pucData, ucState;

	s_usLogTx = 0;
	ucState = pifUart_StartGetTxData(p_uart, &pucData, &s_usLogTx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, pucData, s_usLogTx);
		return TRUE;
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *pucData, ucState;

	if (huart->Instance == USART2) {
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		pifUart_PutRxByte(&s_uart_log, s_ucLogRx);
		HAL_UART_Receive_IT(huart, &s_ucLogRx, 1);
	}
}

// LD2 (PA5, D13 of the Arduino header) is where the scope is clipped on.
void actRealtimeSignal(BOOL state)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 0)) goto fail;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) goto fail;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 50000, NULL)) goto fail;		// 50ms
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64)) goto fail;										// 64 bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) goto fail;										// 128 bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	pifLog_Init();
	// The table of the task command goes out at once, more than the 256 bytes the other examples keep.
	if (!pifLog_AttachUart(&s_uart_log, 1024)) goto fail;											// 1024bytes
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
