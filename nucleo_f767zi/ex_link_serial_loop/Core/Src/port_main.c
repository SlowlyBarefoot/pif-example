#include "main.h"
#include "app_port.h"


#define USE_INTERRUPT
//#define USE_DMA

#ifdef USE_INTERRUPT
	#define UART_FRAME_SIZE		1
#endif
#ifdef USE_DMA
	#define UART_FRAME_SIZE		8
#endif


static PifUart s_uart_log;

static uint8_t s_ucLogRx;
static uint8_t s_ucSerial1Buffer[UART_FRAME_SIZE];
static uint8_t s_ucSerial2Buffer[UART_FRAME_SIZE];

static uint16_t s_usLogTx;
static uint16_t s_usSerial1Tx;
static uint16_t s_usSerial2Tx;


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *pdata, ucState;

	s_usLogTx = 0;
	ucState = pifUart_StartGetTxData(p_uart, &pdata, &s_usLogTx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart3, pdata, s_usLogTx);
		return TRUE;
	}
	return FALSE;
}

static void actLedLState(PifId pid_id, uint32_t state)
{
	(void)pid_id;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static int32_t actPushSwitchAcquire(PifSensor* p_owner)
{
	static GPIO_TypeDef* usPortSwitch[SWITCH_COUNT] = { Switch1_GPIO_Port, Switch2_GPIO_Port };
	static uint16_t usPinSwitch[SWITCH_COUNT] = { Switch1_Pin, Switch2_Pin };
	int index = p_owner->_id - PIF_ID_SWITCH;

	return !HAL_GPIO_ReadPin(usPortSwitch[index], usPinSwitch[index]);
}

static BOOL actUart1StartTransfer(PifUart* p_uart)
{
	uint8_t *pdata, ucState;

	s_usSerial1Tx = UART_FRAME_SIZE;
	ucState = pifUart_StartGetTxData(p_uart, &pdata, &s_usSerial1Tx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
#ifdef USE_INTERRUPT
		HAL_UART_Transmit_IT(&huart4, pdata, s_usSerial1Tx);
#endif

#ifdef USE_DMA
		HAL_UART_Transmit_DMA(&huart4, pdata, s_usSerial1Tx);
#endif
		return TRUE;
	}
	return FALSE;
}

static BOOL actUart2StartTransfer(PifUart* p_uart)
{
	uint8_t *pdata, ucState;

	s_usSerial2Tx = UART_FRAME_SIZE;
	ucState = pifUart_StartGetTxData(p_uart, &pdata, &s_usSerial2Tx);
	if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
#ifdef USE_INTERRUPT
		HAL_UART_Transmit_IT(&huart5, pdata, s_usSerial2Tx);
#endif

#ifdef USE_DMA
		HAL_UART_Transmit_DMA(&huart5, pdata, s_usSerial2Tx);
#endif
		return TRUE;
	}
	return FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *pdata, ucState;

	if (huart->Instance == USART3) {
		ucState = pifUart_EndGetTxData(&s_uart_log, s_usLogTx);
		if (!(ucState & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_usLogTx = 0;
			ucState = pifUart_StartGetTxData(&s_uart_log, &pdata, &s_usLogTx);
			if (ucState & PIF_UART_SEND_DATA_STATE_DATA) {
				HAL_UART_Transmit_IT(huart, pdata, s_usLogTx);
			}
		}
	}
	else if (huart->Instance == UART4) {
		ucState = pifUart_EndGetTxData(&g_serial1, s_usSerial1Tx);
		if (!(ucState & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_usSerial1Tx = UART_FRAME_SIZE;
			ucState = pifUart_StartGetTxData(&g_serial1, &pdata, &s_usSerial1Tx);
			if (ucState & 1) {
#ifdef USE_INTERRUPT
				HAL_UART_Transmit_IT(huart, pdata, s_usSerial1Tx);
#endif

#ifdef USE_DMA
				HAL_UART_Transmit_DMA(huart, pdata, s_usSerial1Tx);
#endif
			}
		}
	}
	else if (huart->Instance == UART5) {
		ucState = pifUart_EndGetTxData(&g_serial2, s_usSerial2Tx);
		if (!(ucState & PIF_UART_SEND_DATA_STATE_EMPTY)) {
			s_usSerial2Tx = UART_FRAME_SIZE;
			ucState = pifUart_StartGetTxData(&g_serial2, &pdata, &s_usSerial2Tx);
			if (ucState & 1) {
#ifdef USE_INTERRUPT
				HAL_UART_Transmit_IT(huart, pdata, s_usSerial2Tx);
#endif

#ifdef USE_DMA
				HAL_UART_Transmit_DMA(huart, pdata, s_usSerial2Tx);
#endif
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) {
		pifUart_PutRxByte(&s_uart_log, s_ucLogRx);
		HAL_UART_Receive_IT(huart, &s_ucLogRx, 1);
	}
	else if (huart->Instance == UART4) {
		pifUart_PutRxData(&g_serial1, s_ucSerial1Buffer, UART_FRAME_SIZE);

#ifdef USE_INTERRUPT
		HAL_UART_Receive_IT(huart, s_ucSerial1Buffer, 1);
#endif
#ifdef USE_DMA
		HAL_UART_Receive_DMA(huart, s_ucSerial1Buffer, UART_FRAME_SIZE);
#endif
	}
	else if (huart->Instance == UART5) {
		pifUart_PutRxData(&g_serial2, s_ucSerial2Buffer, UART_FRAME_SIZE);

#ifdef USE_INTERRUPT
		HAL_UART_Receive_IT(huart, s_ucSerial2Buffer, 1);
#endif
#ifdef USE_DMA
		HAL_UART_Receive_DMA(huart, s_ucSerial2Buffer, UART_FRAME_SIZE);
#endif
	}
}

BOOL portSetup()
{
	int line;

	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) {
		line = __LINE__;
		goto fail;
	}

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) {	// 1000us
		line = __LINE__;
		goto fail;
	}

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart3.Init.BaudRate)) {
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD, 1000, "UartLog")) {			// 1ms
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AllocTxBuffer(&s_uart_log, 64)) {
		line = __LINE__;
		goto fail;
	}
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart3, &s_ucLogRx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) {
		line = __LINE__;
		goto fail;
	}

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) {
		line = __LINE__;
		goto fail;
	}

	for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_link_test[i].push_switch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) {
			line = __LINE__;
			goto fail;
		}
	}

	if (!pifUart_Init(&g_serial1, PIF_ID_AUTO, huart4.Init.BaudRate)) {
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AttachTask(&g_serial1, TM_PERIOD, 1000, "UartSerial1")) {		// 1ms
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AllocRxBuffer(&g_serial1, 64, 10)) {								// 10%
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AllocTxBuffer(&g_serial1, 64)) {
		line = __LINE__;
		goto fail;
	}
	g_serial1.act_start_transfer = actUart1StartTransfer;

#ifdef USE_INTERRUPT
	HAL_UART_Receive_IT(&huart4, s_ucSerial1Buffer, UART_FRAME_SIZE);
#endif
#ifdef USE_DMA
	if (!pifUart_SetFrameSize(&g_serial1, UART_FRAME_SIZE)) {
		line = __LINE__;
		goto fail;
	}

	HAL_UART_Receive_DMA(&huart4, s_ucSerial1Buffer, UART_FRAME_SIZE);
#endif

	if (!pifUart_Init(&g_serial2, PIF_ID_AUTO, huart5.Init.BaudRate)) {
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AttachTask(&g_serial2, TM_PERIOD, 1000, "UartSerial2")) {		// 1ms
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AllocRxBuffer(&g_serial2, 64, 10)) {								// 10%
		line = __LINE__;
		goto fail;
	}
	if (!pifUart_AllocTxBuffer(&g_serial2, 64)) {
		line = __LINE__;
		goto fail;
	}
	g_serial2.act_start_transfer = actUart2StartTransfer;

#ifdef USE_INTERRUPT
	HAL_UART_Receive_IT(&huart5, s_ucSerial1Buffer, UART_FRAME_SIZE);
#endif
#ifdef USE_DMA
	if (!pifUart_SetFrameSize(&g_serial2, UART_FRAME_SIZE)) {
		line = __LINE__;
		goto fail;
	}

	HAL_UART_Receive_DMA(&huart5, s_ucSerial2Buffer, UART_FRAME_SIZE);
#endif
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Port Setup failed. %d\n", line);
	pif_Exit();
	return FALSE;
}
