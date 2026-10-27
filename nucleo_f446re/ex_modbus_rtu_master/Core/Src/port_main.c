#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint8_t s_log_rx;
static uint16_t s_log_tx;

static uint8_t s_modbus_rx;
static uint16_t s_modbus_tx;


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static BOOL actLogStartTransfer(PifUart *p_uart)
{
	uint8_t *p_data, state;

	state = pifUart_StartGetTxData(p_uart, &p_data, &s_log_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, p_data, s_log_tx);
		return TRUE;
	}
	return FALSE;
}

static BOOL actModbusStartTransfer(PifUart *p_uart)
{
	uint8_t *p_data, state;

	state = pifUart_StartGetTxData(p_uart, &p_data, &s_modbus_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart4, p_data, s_modbus_tx);
		return TRUE;
	}
	return FALSE;
}

static void actModbusDirection(PifUartDirection direction)
{
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, direction);
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
	else if (huart->Instance == UART4) {
		state = pifUart_EndGetTxData(&g_uart_modbus, s_modbus_tx);
		if (!(state & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_modbus_tx = 0;
			state = pifUart_StartGetTxData(&g_uart_modbus, &p_data, &s_modbus_tx);
			if (state & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, p_data, s_modbus_tx);
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		pifUart_PutRxByte(&s_uart_log, s_log_rx);
		HAL_UART_Receive_IT(huart, &s_log_rx, 1);
	}
	else if (huart->Instance == UART4) {
		pifUart_PutRxByte(&g_uart_modbus, s_modbus_rx);
		HAL_UART_Receive_IT(huart, &s_modbus_rx, 1);
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;					// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) return FALSE;		// 200ms
	if (!pifUart_AllocRxBuffer(&s_uart_log, 32)) return FALSE;												// 32bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;												// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_log_rx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;													// 256bytes

	if (!pifUart_Init(&g_uart_modbus, PIF_ID_AUTO, huart4.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&g_uart_modbus, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxModbus")) return FALSE;
	if (!pifUart_AttachRxTask(&g_uart_modbus, PIF_ID_AUTO, TM_PERIOD, 5000, "UartRxModbus")) return FALSE;	// 5ms
	if (!pifUart_AllocRxBuffer(&g_uart_modbus, 256)) return FALSE;											// 256bytes
	if (!pifUart_AllocTxBuffer(&g_uart_modbus, 256)) return FALSE;											// 256bytes
	g_uart_modbus.act_start_transfer = actModbusStartTransfer;
	pifUart_AttachActDirection(&g_uart_modbus, actModbusDirection, UD_RX);

	HAL_UART_Receive_IT(&huart4, &s_modbus_rx, 1);

	if (!pifModbusRtuMaster_Init(&g_modbus_master, PIF_ID_AUTO)) return FALSE;

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
	return TRUE;
}
