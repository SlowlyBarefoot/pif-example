#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint8_t s_ucLogRx;
static uint16_t s_usLogTx;

static uint8_t s_gps_rx;


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
	else if (huart->Instance == USART6) {
		pifUart_PutRxByte(&g_uart_gps, s_gps_rx);
		HAL_UART_Receive_IT(huart, &s_gps_rx, 1);
	}
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	static uint32_t state;

	(void)usPifId;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, unState & 1);
	if (unState != state) state = unState;
}

static void _actGpioWrite(uint16_t port, SWITCH state)
{
	static BOOL sw[3];

	if (state) {
		switch (port) {
		case PIF_ID_USER(0):
			HAL_GPIO_WritePin(LOG_TX_GPIO_Port, LOG_TX_Pin, sw[1]);
			sw[1] ^= 1;
			break;
		}
	}
	else {
		switch (port) {
		case PIF_ID_USER(0):
			HAL_GPIO_WritePin(LOG_TX_GPIO_Port, LOG_RX_Pin, sw[0]);
			sw[0] ^= 1;
			break;

		case PIF_ID_USER(1):
			HAL_GPIO_WritePin(GPS_RX_GPIO_Port, GPS_RX_Pin, sw[2]);
			sw[2] ^= 1;
			break;
		}
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	pif_act_gpio_write = _actGpioWrite;

	if (!pifUart_Init(&s_uart_log, PIF_ID_USER(0), huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) return FALSE;	// 200ms
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64)) return FALSE;											// 64 bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;											// 256 bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 512)) return FALSE;												// 512bytes

	if (!pifUart_Init(&g_uart_gps, PIF_ID_USER(1), huart6.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachRxTask(&g_uart_gps, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxGPS")) return FALSE;	// 200ms
	if (!pifUart_AllocRxBuffer(&g_uart_gps, 128)) return FALSE;											// 256 bytes

	HAL_UART_Receive_IT(&huart6, &s_gps_rx, 1);

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) return FALSE;
	return TRUE;
}
