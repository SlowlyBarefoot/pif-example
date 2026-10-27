#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;
static SWITCH s_echo_state;


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

static void actHcSr04Trigger(SWITCH state)
{
	HAL_GPIO_WritePin(SONAR_TRIGGER_GPIO_Port, SONAR_TRIGGER_Pin, state);
    s_echo_state = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	s_echo_state ^= 1;
	pifHcSr04_sigReceiveEcho(&g_hcsr04, s_echo_state);
}

static uint32_t taskLedToggle(PifTask* p_task)
{
	static BOOL swLed = OFF;

	(void)p_task;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, swLed);
	swLed ^= 1;
	return 0;
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AllocTxBuffer(&s_uart_log, 64)) return FALSE;											// 64bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

	if (!pifHcSr04_Init(&g_hcsr04, PIF_ID_AUTO, NULL)) return FALSE;
	g_hcsr04.act_trigger = actHcSr04Trigger;

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return FALSE;	// 500ms
	return TRUE;
}
