/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "linker.h"

#include "stm32f4xx_ll_gpio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TASK_SIZE				10
#define TIMER_1MS_SIZE			3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#if LCD_TYPE == LCD_2_2_INCH_SPI
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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

static BOOL actPen()
{
	return !HAL_GPIO_ReadPin(PEN_GPIO_Port, PEN_Pin);
}

static void actTransfer(PifSpiDevice *p_owner, uint8_t* p_write, uint8_t* p_read, size_t size)
{
	(void)p_owner;

	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, OFF);
	HAL_GPIO_WritePin(T_NSS_GPIO_Port, T_NSS_Pin, OFF);
	HAL_SPI_Transmit(&hspi2, p_write, 1, 10);
	HAL_SPI_Receive(&hspi2, p_read, size, 10);
	HAL_GPIO_WritePin(T_NSS_GPIO_Port, T_NSS_Pin, ON);
	HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, ON);
}

#endif

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
#if LCD_TYPE == LCD_2_2_INCH_SPI
  const char *lcd_name = "2.2 Inch LCD SPI";
#elif LCD_TYPE == LCD_3_2_INCH
  const char *lcd_name = "  3.2 Inch LCD  ";
#endif
  int line;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
#if LCD_TYPE == LCD_3_2_INCH
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, ON);
#endif

  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  pif_Init(micros);

  if (!pifTaskManager_Init(TASK_SIZE)) { line = __LINE__; goto fail; }

  if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) { line = __LINE__; goto fail; }		// 1000us

  if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) { line = __LINE__; goto fail; }
  if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) { line = __LINE__; goto fail; }					// 1ms
  if (!pifUart_AllocRxBuffer(&s_uart_log, 64, 100)) { line = __LINE__; goto fail; }									// 100%
  if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) { line = __LINE__; goto fail; }
  s_uart_log.act_start_transfer = actLogStartTransfer;

  HAL_UART_Receive_IT(&huart2, &s_log_rx, 1);

  pifLog_Init();
  if (!pifLog_AttachUart(&s_uart_log)) { line = __LINE__; goto fail; }

  if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) { line = __LINE__; goto fail; }			// 2EA

#if LCD_TYPE == LCD_2_2_INCH_SPI
  if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_8BIT_I)) { line = __LINE__; goto fail; }
  g_ili9341.parent.act_backlight = actLcdBackLight;

  if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, NULL, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#elif LCD_TYPE == LCD_3_2_INCH
  if (!pifIli9341_Init(&g_ili9341, PIF_ID_AUTO, ILI9341_IF_MCU_16BIT_I)) { line = __LINE__; goto fail; }
  g_ili9341.parent.act_backlight = actLcdBackLight;

  if (!pifSpiPort_Init(&g_spi_port, PIF_ID_AUTO, 1, 16)) { line = __LINE__; goto fail; }
  g_spi_port.act_transfer = actTransfer;

  if (!pifTsc2046_Init(&g_tsc2046, PIF_ID_AUTO, &g_ili9341.parent, 3761, 229, 3899, 420, &g_spi_port, actPen)) { line = __LINE__; goto fail; }
  if (!pifIli9341_AttachActParallel(&g_ili9341, actLcdReset, actLcdChipSelect, actLcdReadCmd, actLcdWriteCmd, actLcdWriteData, actLcdWriteRepeat)) { line = __LINE__; goto fail; }
#endif
  pifIli9341_Setup(&g_ili9341, lcd_setup, lcd_rotation);

  if (!appSetup()) { line = __LINE__; goto fail; }

  pifLog_Print(LT_NONE, "\n\n****************************************\n");
  pifLog_Print(LT_NONE, "***          ex_tft_ili9341          ***\n");
  pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
  pifLog_Printf(LT_NONE, "***         %s         ***\n", lcd_name);
  pifLog_Print(LT_NONE, "****************************************\n");
  pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	pifTaskManager_Loop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  return 0;

fail:
  pifLog_Printf(LT_INFO, "Initial failed. %d\n", line);
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|T_CS_Pin|LCD_D1_Pin|LCD_D10_Pin
                          |LCD_D11_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LD2_Pin
                          |LCD_D7_Pin|LCD_D0_Pin|LCD_D2_Pin|LCD_D8_Pin
                          |LCD_D9_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D12_Pin|LCD_D13_Pin|LCD_D6_Pin
                          |LCD_D14_Pin|LCD_D15_Pin|LCD_D5_Pin|LCD_D4_Pin
                          |LCD_D3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RST_Pin T_CS_Pin LCD_D1_Pin LCD_D10_Pin
                           LCD_D11_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|T_CS_Pin|LCD_D1_Pin|LCD_D10_Pin
                          |LCD_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LD2_Pin
                           LCD_D7_Pin LCD_D0_Pin LCD_D2_Pin LCD_D8_Pin
                           LCD_D9_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LD2_Pin
                          |LCD_D7_Pin|LCD_D0_Pin|LCD_D2_Pin|LCD_D8_Pin
                          |LCD_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D12_Pin LCD_D13_Pin LCD_D6_Pin
                           LCD_D14_Pin LCD_D15_Pin LCD_D5_Pin LCD_D4_Pin
                           LCD_D3_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D12_Pin|LCD_D13_Pin|LCD_D6_Pin
                          |LCD_D14_Pin|LCD_D15_Pin|LCD_D5_Pin|LCD_D4_Pin
                          |LCD_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PEN_Pin */
  GPIO_InitStruct.Pin = PEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PEN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
