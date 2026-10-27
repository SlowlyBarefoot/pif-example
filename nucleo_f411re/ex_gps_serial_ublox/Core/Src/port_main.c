#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint8_t s_ucLogRx;
static uint16_t s_usLogTx;

static uint8_t s_gps_rx;
static uint16_t s_gps_tx;


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *pucData, ucState;

	ucState = pifUart_StartGetTxData(p_uart, &pucData, &s_usLogTx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, pucData, s_usLogTx);
		return TRUE;
	}
	return FALSE;
}

static BOOL actGpsSetBaudrate(PifUart* p_uart, uint32_t baudrate)
{
	if (huart6.Init.BaudRate == baudrate) return TRUE;

	HAL_UART_Abort_IT(&huart6);
	pifUart_AbortRx(&g_uart_gps);
	HAL_UART_DeInit(&huart6);
	huart6.Init.BaudRate = baudrate;
	if (HAL_UART_Init(&huart6) != HAL_OK) return FALSE;
	if (HAL_UART_Receive_IT(&huart6, &s_gps_rx, 1) != HAL_OK) return FALSE;
	return TRUE;
}

static BOOL actGpsStartTransfer(PifUart* p_uart)
{
	uint8_t *p_data, state;

	state = pifUart_StartGetTxData(p_uart, &p_data, &s_gps_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart6, p_data, s_gps_tx);
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
	else if (huart->Instance == USART6) {
		ucState = pifUart_EndGetTxData(&g_uart_gps, s_gps_tx);
		if (!(ucState & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_gps_tx = 0;
			ucState = pifUart_StartGetTxData(&g_uart_gps, &pucData, &s_gps_tx);
			if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, pucData, s_gps_tx);
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
	else if (huart->Instance == USART6) {
		pifUart_PutRxByte(&g_uart_gps, s_gps_rx);
		if (g_print_data == 2) HAL_UART_Transmit_IT(&huart2, &s_gps_rx, 1);
		HAL_UART_Receive_IT(huart, &s_gps_rx, 1);
	}
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	static uint32_t state;

	(void)usPifId;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, unState & 1);
	if (unState != state) {
		if (g_print_data == 1) pifLog_Printf(LT_INFO, "LED State=%u", unState);
		state = unState;
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) return FALSE;	// 200ms
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64)) return FALSE;											// 64bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;											// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 128)) return FALSE;												// 128bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) return FALSE;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO, huart6.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&g_uart_gps, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxGPS")) return FALSE;
	if (!pifUart_AttachRxTask(&g_uart_gps, PIF_ID_AUTO, TM_PERIOD, 100000, "UartRxGPS")) return FALSE;	// 100ms
	if (!pifUart_AllocRxBuffer(&g_uart_gps, 128)) return FALSE;											// 128bytes
	if (!pifUart_AllocTxBuffer(&g_uart_gps, 64)) return FALSE;											// 64bytes
	g_uart_gps.act_start_transfer = actGpsStartTransfer;
	g_uart_gps.act_set_baudrate = actGpsSetBaudrate;

	HAL_UART_Receive_IT(&huart6, &s_gps_rx, 1);
	return TRUE;
}
