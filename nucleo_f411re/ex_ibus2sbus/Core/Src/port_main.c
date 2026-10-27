#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_log_tx;
static uint8_t s_ibus_rx;
static uint16_t s_sbus_tx;


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *p_data, state;

	state = pifUart_StartGetTxData(p_uart, &p_data, &s_log_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, p_data, s_log_tx);
		return TRUE;
	}
	return FALSE;
}

static BOOL actSbusStartTransfer(PifUart* p_uart)
{
	uint8_t *p_data, state;

	state = pifUart_StartGetTxData(p_uart, &p_data, &s_sbus_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart1, p_data, s_sbus_tx);
		return TRUE;
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *p_data, state;

	if (huart->Instance == USART1) {
		state = pifUart_EndGetTxData(&g_uart_sbus, s_sbus_tx);
		if (!(state & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_sbus_tx = 0;
			state = pifUart_StartGetTxData(&g_uart_sbus, &p_data, &s_sbus_tx);
			if (state & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, p_data, s_sbus_tx);
			}
		}
	}
	else if (huart->Instance == USART2) {
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART6) {
		pifUart_PutRxByte(&g_uart_ibus, s_ibus_rx);
		HAL_UART_Receive_IT(huart, &s_ibus_rx, 1);
	}
}

static void actLedLState(PifId usPifId, uint32_t state)
{
	(void)usPifId;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static void _actGpioWrite(uint16_t port, SWITCH state)
{
	if (state) {	// TX
		switch (port) {
		case PIF_ID_USER(0):
			HAL_GPIO_TogglePin(LOG_TX_GPIO_Port, LOG_TX_Pin);
			break;

		case PIF_ID_USER(2):
			HAL_GPIO_TogglePin(SBUS_TX_GPIO_Port, SBUS_TX_Pin);
			break;
		}
	}
	else {			// RX
		switch (port) {
		case PIF_ID_USER(1):
			HAL_GPIO_TogglePin(IBUS_RX_GPIO_Port, IBUS_RX_Pin);
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
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) return FALSE;
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;											// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

	if (!pifUart_Init(&g_uart_ibus, PIF_ID_USER(1), huart6.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachRxTask(&g_uart_ibus, PIF_ID_AUTO, TM_PERIOD, 3000, "UartIbus")) return FALSE;	// 3ms
	if (!pifUart_AllocRxBuffer(&g_uart_ibus, 64)) return FALSE;											// 64bytes

	HAL_UART_Receive_IT(&huart6, &s_ibus_rx, 1);

	if (!pifUart_Init(&g_uart_sbus, PIF_ID_USER(2), huart1.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&g_uart_sbus, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartSbus")) return FALSE;
	if (!pifUart_AllocTxBuffer(&g_uart_sbus, 64)) return FALSE;											// 64bytes
	g_uart_sbus.act_start_transfer = actSbusStartTransfer;

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) return FALSE;
	return TRUE;
}
