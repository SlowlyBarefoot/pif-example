#include "main.h"
#include "app_port.h"
#include "stm32f4xx_ll_gpio.h"


#if LCD_TYPE == LCD_2_2_INCH_SPI || LCD_TYPE == LCD_2_4_INCH
	const uint8_t lcd_setup[] = {
		ILI9341_CMD_POWER_CTRL_A, 			5, 0x39, 0x2C, 0x00, 0x34, 0x02,
		ILI9341_CMD_POWER_CTRL_B, 			3, 0x00, 0xC1, 0x30,
		ILI9341_CMD_DRIVER_TIM_CTRL_A1, 	3, 0x85, 0x00, 0x78,
		ILI9341_CMD_DRIVER_TIM_CTRL_B, 		2, 0x00, 0x00,
		ILI9341_CMD_POWER_SEQ_CTRL, 		4, 0x64, 0x03, 0x12, 0x81,
		ILI9341_CMD_PUMP_RATIO_CTRL, 		1, 0x20,
		ILI9341_CMD_POWER_CTRL1, 			1, 0x23,
		ILI9341_CMD_POWER_CTRL2, 			1, 0x10,
		ILI9341_CMD_VCOM_CTRL1, 			2, 0x3E, 0x28,
		ILI9341_CMD_VCOM_CTRL2, 			1, 0x86,
		ILI9341_CMD_MEM_ACCESS_CTRL, 		1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_CMD_PIXEL_FORMAT_SET, 		1, (ILI9341_PF_16BIT << 4) | ILI9341_PF_16BIT,
		ILI9341_CMD_FRAME_RATE_CTRL_NORMAL,	2, 0x00, 0x18,
		ILI9341_CMD_DISP_FUNC_CTRL, 		3, 0x08, 0x82, 0x27,
		ILI9341_CMD_NOP
	};
	const uint8_t lcd_rotation[] = {
		ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
	};
#elif LCD_TYPE == LCD_3_2_INCH
	const uint8_t lcd_setup[] = {
		ILI9341_CMD_POWER_CTRL_B, 			 3, 0x00, 0xD9, 0x30,
		ILI9341_CMD_POWER_SEQ_CTRL, 		 4, 0x64, 0x03, 0x12, 0x81,
		ILI9341_CMD_DRIVER_TIM_CTRL_A1, 	 3, 0x85, 0x10, 0x7A,
		ILI9341_CMD_POWER_CTRL_A, 			 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
		ILI9341_CMD_PUMP_RATIO_CTRL, 		 1, 0x20,
		ILI9341_CMD_DRIVER_TIM_CTRL_B, 		 2, 0x00, 0x00,
		ILI9341_CMD_POWER_CTRL1, 			 1, 0x1B,
		ILI9341_CMD_POWER_CTRL2, 			 1, 0x12,
		ILI9341_CMD_VCOM_CTRL1, 			 2, 0x26, 0x26,
		ILI9341_CMD_VCOM_CTRL2, 			 1, 0xB0,
		ILI9341_CMD_MEM_ACCESS_CTRL, 		 1, ILI9341_MADCTL_BGR,
		ILI9341_CMD_PIXEL_FORMAT_SET, 		 1, (ILI9341_PF_16BIT << 4) | ILI9341_PF_16BIT,
		ILI9341_CMD_FRAME_RATE_CTRL_NORMAL,  2, 0x00, 0x1A,
		ILI9341_CMD_DISP_FUNC_CTRL, 	 	 2, 0x0A, 0xA2,
		ILI9341_CMD_ENABLE_3G, 			 	 1, 0x00,
		ILI9341_CMD_GAMMA_SET, 			 	 1, 0x01,
		ILI9341_CMD_POS_GAMMA_CORRECT, 		15, 0x1F, 0x24, 0x24, 0x0D, 0x12, 0x09, 0x52, 0xB7, 0x3F, 0x0C, 0x15, 0x06, 0x0E, 0x08, 0x00,
		ILI9341_CMD_NEG_GAMMA_CORRECT, 		15, 0x00, 0x1B, 0x1B, 0x02, 0x0E, 0x06, 0x2E, 0x48, 0x3F, 0x03, 0x0A, 0x09, 0x31, 0x37, 0x1F,
		ILI9341_CMD_PAGE_ADDR_SET, 			 4, 0x00, 0x00, 0x01, 0x3F,
		ILI9341_CMD_COL_ADDR_SET, 			 4, 0x00, 0x00, 0x00, 0xEF,
		ILI9341_CMD_NOP
	};
	const uint8_t lcd_rotation[] = {
		ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MV | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR,
		ILI9341_MADCTL_MY | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR
	};
#endif

static PifUart s_uart_log;

static uint16_t s_log_tx;
static uint8_t s_log_rx;


static uint32_t micros()
{
    return htim2.Instance->CNT;
}

static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state & 1);
}

static BOOL actLogStartTransfer(PifUart* p_uart)
{
	uint8_t *p_data, state;

	s_log_tx = 0;
	state = pifUart_StartGetTxData(p_uart, &p_data, &s_log_tx);
	if (state & PIF_UART_SEND_DATA_STATE_DATA) {
		HAL_UART_Transmit_IT(&huart2, p_data, s_log_tx);
		return TRUE;
	}
	return FALSE;
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
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		pifUart_PutRxByte(&s_uart_log, s_log_rx);
		HAL_UART_Receive_IT(huart, &s_log_rx, 1);
	}
}

static void actLcdReset()
{
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, OFF);
	pif_Delay1ms(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, ON);
	pif_Delay1ms(50);
}

static void actLcdChipSelect(SWITCH sw)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, sw ? OFF : ON);
}

#if LCD_TYPE == LCD_2_2_INCH_SPI

static void _LcdWritBus(uint32_t d)
{
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&d, 1, 1);
}

#else

static uint32_t _LcdReadBus()
{
	uint32_t d;

	HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, OFF);
	pif_Delay1us(1);
	d = HAL_GPIO_ReadPin(LCD_D0_GPIO_Port, LCD_D0_Pin) << 0;
	d |= HAL_GPIO_ReadPin(LCD_D1_GPIO_Port, LCD_D1_Pin) << 1;
	d |= HAL_GPIO_ReadPin(LCD_D2_GPIO_Port, LCD_D2_Pin) << 2;
	d |= HAL_GPIO_ReadPin(LCD_D3_GPIO_Port, LCD_D3_Pin) << 3;
	d |= HAL_GPIO_ReadPin(LCD_D4_GPIO_Port, LCD_D4_Pin) << 4;
	d |= HAL_GPIO_ReadPin(LCD_D5_GPIO_Port, LCD_D5_Pin) << 5;
	d |= HAL_GPIO_ReadPin(LCD_D6_GPIO_Port, LCD_D6_Pin) << 6;
	d |= HAL_GPIO_ReadPin(LCD_D7_GPIO_Port, LCD_D7_Pin) << 7;
#if LCD_TYPE == LCD_3_2_INCH
	d |= HAL_GPIO_ReadPin(LCD_D8_GPIO_Port, LCD_D8_Pin) << 8;
	d |= HAL_GPIO_ReadPin(LCD_D9_GPIO_Port, LCD_D9_Pin) << 9;
	d |= HAL_GPIO_ReadPin(LCD_D10_GPIO_Port, LCD_D10_Pin) << 10;
	d |= HAL_GPIO_ReadPin(LCD_D11_GPIO_Port, LCD_D11_Pin) << 11;
	d |= HAL_GPIO_ReadPin(LCD_D12_GPIO_Port, LCD_D12_Pin) << 12;
	d |= HAL_GPIO_ReadPin(LCD_D13_GPIO_Port, LCD_D13_Pin) << 13;
	d |= HAL_GPIO_ReadPin(LCD_D14_GPIO_Port, LCD_D14_Pin) << 14;
	d |= HAL_GPIO_ReadPin(LCD_D15_GPIO_Port, LCD_D15_Pin) << 15;
#endif
	HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, ON);
	return d;
}

static void _LcdWritBus(uint32_t d)
{
	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (d >> 0) & 1);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, (d >> 1) & 1);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, (d >> 2) & 1);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, (d >> 3) & 1);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (d >> 4) & 1);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (d >> 5) & 1);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (d >> 6) & 1);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (d >> 7) & 1);
#if LCD_TYPE == LCD_3_2_INCH
	HAL_GPIO_WritePin(LCD_D8_GPIO_Port, LCD_D8_Pin, (d >> 8) & 1);
	HAL_GPIO_WritePin(LCD_D9_GPIO_Port, LCD_D9_Pin, (d >> 9) & 1);
	HAL_GPIO_WritePin(LCD_D10_GPIO_Port, LCD_D10_Pin, (d >> 10) & 1);
	HAL_GPIO_WritePin(LCD_D11_GPIO_Port, LCD_D11_Pin, (d >> 11) & 1);
	HAL_GPIO_WritePin(LCD_D12_GPIO_Port, LCD_D12_Pin, (d >> 12) & 1);
	HAL_GPIO_WritePin(LCD_D13_GPIO_Port, LCD_D13_Pin, (d >> 13) & 1);
	HAL_GPIO_WritePin(LCD_D14_GPIO_Port, LCD_D14_Pin, (d >> 14) & 1);
	HAL_GPIO_WritePin(LCD_D15_GPIO_Port, LCD_D15_Pin, (d >> 15) & 1);
#endif
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, OFF);
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, ON);
}

static void actLcdReadCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, OFF);
	_LcdWritBus(cmd);

	if (size) {
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, ON);
		LL_GPIO_SetPinMode(LCD_D0_GPIO_Port, LCD_D0_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D1_GPIO_Port, LCD_D1_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D2_GPIO_Port, LCD_D2_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D3_GPIO_Port, LCD_D3_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D4_GPIO_Port, LCD_D4_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D5_GPIO_Port, LCD_D5_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_INPUT);
#if LCD_TYPE == LCD_3_2_INCH
		LL_GPIO_SetPinMode(LCD_D8_GPIO_Port, LCD_D8_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D9_GPIO_Port, LCD_D9_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D10_GPIO_Port, LCD_D10_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D11_GPIO_Port, LCD_D11_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D12_GPIO_Port, LCD_D12_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D13_GPIO_Port, LCD_D13_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D14_GPIO_Port, LCD_D14_Pin, LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinMode(LCD_D15_GPIO_Port, LCD_D15_Pin, LL_GPIO_MODE_INPUT);
#endif
		pif_Delay1us(1);
		for (uint32_t i = 0; i < size; i++) {
			p_data[i] = _LcdReadBus();
		}
		LL_GPIO_SetPinMode(LCD_D0_GPIO_Port, LCD_D0_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D1_GPIO_Port, LCD_D1_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D2_GPIO_Port, LCD_D2_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D3_GPIO_Port, LCD_D3_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D4_GPIO_Port, LCD_D4_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D5_GPIO_Port, LCD_D5_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_OUTPUT);
#if LCD_TYPE == LCD_3_2_INCH
		LL_GPIO_SetPinMode(LCD_D8_GPIO_Port, LCD_D8_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D9_GPIO_Port, LCD_D9_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D10_GPIO_Port, LCD_D10_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D11_GPIO_Port, LCD_D11_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D12_GPIO_Port, LCD_D12_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D13_GPIO_Port, LCD_D13_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D14_GPIO_Port, LCD_D14_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(LCD_D15_GPIO_Port, LCD_D15_Pin, LL_GPIO_MODE_OUTPUT);
#endif
	}
}

#endif

static void actLcdWriteCmd(PifTftLcdCmd cmd, uint32_t* p_data, uint32_t size)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, OFF);
	_LcdWritBus(cmd);

	if (size) {
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, ON);
		for (uint32_t i = 0; i < size; i++) {
			_LcdWritBus(p_data[i]);
		}
	}
}

static void actLcdWriteData(uint32_t* p_data, uint32_t size)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, ON);
	for (uint32_t i = 0; i < size; i++) {
		_LcdWritBus(p_data[i]);
	}
}

static void actLcdWriteRepeat(uint32_t* p_data, uint8_t size, uint32_t len)
{
	uint8_t p = 0;

	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, ON);
	for (uint32_t i = 0; i < size * len; i++) {
		_LcdWritBus(p_data[p]);
		p++;
		if (p >= size) p = 0;
	}
}

#if LCD_TYPE == LCD_2_2_INCH_SPI || LCD_TYPE == LCD_3_2_INCH

static void actLcdBackLight(uint8_t level)
{
	htim3.Instance->CCR3 = level * 1000 / 100;
}

#endif

#if LCD_TYPE == LCD_3_2_INCH

static BOOL actPen(void)
{
	return !HAL_GPIO_ReadPin(PEN_GPIO_Port, PEN_Pin);
}

static void actTransfer(PifSpiDevice *p_owner, uint8_t* p_write, uint8_t* p_read, size_t size)
{
	(void)p_owner;

	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, OFF);
	HAL_GPIO_WritePin(T_NSS_GPIO_Port, T_NSS_Pin, OFF);
	HAL_SPI_TransmitReceive(&hspi2, p_write, p_read, size, 10);
	HAL_GPIO_WritePin(T_NSS_GPIO_Port, T_NSS_Pin, ON);
	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, ON);
}

#endif

BOOL portSetup()
{
	int line;

#if LCD_TYPE == LCD_3_2_INCH
	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, ON);
#endif

	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) { line = __LINE__; goto fail; }

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) { line = __LINE__; goto fail; }
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) { line = __LINE__; goto fail; }
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) { line = __LINE__; goto fail; }	// 200ms
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64)) { line = __LINE__; goto fail; }											// 64bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) { line = __LINE__; goto fail; }											// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_log_rx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) { line = __LINE__; goto fail; }												// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) { line = __LINE__; goto fail; }				// 2EA

#if LCD_TYPE == LCD_2_2_INCH_SPI
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) { line = __LINE__; goto fail; }
	g_ili9341.parent.act_backlight = actLcdBackLight;

	if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, NULL, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#elif LCD_TYPE == LCD_2_4_INCH
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) { line = __LINE__; goto fail; }
	if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#elif LCD_TYPE == LCD_3_2_INCH
	if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_16BIT_I)) { line = __LINE__; goto fail; }
	g_ili9341.parent.act_backlight = actLcdBackLight;

	if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1)) { line = __LINE__; goto fail; }
	g_spi_port.act_transfer = actTransfer;

	if (!pifTsc2046_Init(&g_tsc2046, PIF_ID_AUTO, &g_ili9341.parent, 3761, 229, 3899, 420, &g_spi_port, NULL, actPen)) { line = __LINE__; goto fail; }
	if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#endif
	pifIli9341_Setup(&g_ili9341, lcd_setup, lcd_rotation);
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Initial failed. %d\n", line);
	pif_Exit();
	return FALSE;
}
