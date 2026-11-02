#include "main.h"
#include "app_port.h"


static PifUart s_uart_log;

static uint16_t s_usLogTx;
static uint8_t s_ucLogRx;


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static BOOL actLogStartTransfer(PifUart *p_uart)
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

// TIM3 counts microseconds over a 20 ms period, so the compare value is the pulse itself. With the compare
// preloaded a new pulse takes effect at the next period, and 0 holds the output low, which detaches the servo.
static void actServoControl(uint8_t index, uint16_t pulse_us)
{
	(void)index;

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse_us);
}

// MX_ADC1_Init() sets up one regular conversion, so the channel is switched into rank 1 before each read.
// At PCLK2 / 4 = 21 MHz and 84 cycles a sample, it takes about 5 us a channel.
static uint16_t _adcRead(uint32_t channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	uint16_t value = 0;

	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) return 0;

	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) {
		value = HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
	return value;
}

// VRx is on IN0 (A0) and VRy on IN1 (A1).
static void actJoystickRead(PifJoystick *p_owner, int16_t *p_x, int16_t *p_y)
{
	(void)p_owner;

	*p_x = _adcRead(ADC_CHANNEL_0);
	*p_y = _adcRead(ADC_CHANNEL_1);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	// The compare is 0 from MX_TIM3_Init(), so the output stays low until the servo is set.
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return FALSE;		// 200ms
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AllocRxBuffer(&s_uart_log, 256)) return FALSE;										// 256 bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;										// 128 bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	if (!pifLog_InitHeap(1024)) return FALSE;														// 1024bytes
	if (!pifLog_AttachUart(&s_uart_log, 128)) return FALSE;											// 128bytes

	if (!pifServo_Init(&g_servo, PIF_ID_AUTO, SERVO_COUNT, SERVO_PERIOD, actServoControl)) return FALSE;

	if (!pifJoystick_Init(&g_joystick, PIF_ID_AUTO, 0, JOYSTICK_RAW_MAX, actJoystickRead)) return FALSE;

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
	return TRUE;
}
