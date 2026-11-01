#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;
static uint8_t s_ucLogRx;


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

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static void actDs1302Write(PifDs1302Pin pin, SWITCH state)
{
	switch (pin) {
	case DS1302_PIN_CE:
		HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, state);
		break;

	case DS1302_PIN_SCLK:
		HAL_GPIO_WritePin(DS1302_SCLK_GPIO_Port, DS1302_SCLK_Pin, state);
		break;

	case DS1302_PIN_IO:
		HAL_GPIO_WritePin(DS1302_IO_GPIO_Port, DS1302_IO_Pin, state);
		break;
	}
}

static SWITCH actDs1302Read()
{
	return HAL_GPIO_ReadPin(DS1302_IO_GPIO_Port, DS1302_IO_Pin);
}

static void actDs1302IoMode(BOOL input)
{
	GPIO_InitTypeDef init = {0};

	init.Pin = DS1302_IO_Pin;
	init.Mode = input ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DS1302_IO_GPIO_Port, &init);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return FALSE;		// 200ms
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AllocRxBuffer(&s_uart_log, 256)) return FALSE;										// 256 bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;										// 128 bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	if (!pifLog_Init()) return FALSE;
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;											// 256bytes

	// The default wait of 1us keeps SCLK under 500kHz, which the chip takes at any supply.
	if (!pifDs1302_Init(&g_ds1302, PIF_ID_AUTO, actDs1302Write, actDs1302Read, actDs1302IoMode)) return FALSE;

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
	return TRUE;
}
