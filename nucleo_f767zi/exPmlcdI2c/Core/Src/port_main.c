#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;


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

static void actLedLState(PifId usPifId, uint32_t unState)
{
  (void)usPifId;

  HAL_GPIO_WritePin(GPIOB, LD2_Pin, unState & 1);
}

static PifI2cReturn actI2cWrite(PifI2cDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, size_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Write_IT(&hi2c1, p_owner->addr << 1, iaddr, isize, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Transmit_IT(&hi2c1, p_owner->addr << 1, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C1) {
		pifI2cPort_sigEndTransfer(&g_i2c_port, TRUE);
	}
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart3.Init.BaudRate)) goto fail;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	if (!pifUart_AllocTxBuffer(&s_uart_log, 64)) goto fail;										// 64bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;											// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) goto fail;

	if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1)) goto fail;
	g_i2c_port.act_write = actI2cWrite;
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
