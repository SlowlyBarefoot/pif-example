/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "app_main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static PifUart s_uart_log;

static uint16_t s_usLogTx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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

static void actLedLState(PifId usPifId, uint32_t unState)
{
    (void)usPifId;

    HAL_GPIO_WritePin(GPIOA, LD2_Pin, unState & 1);
}

#ifdef USE_POLLING

static PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Read(&hi2c1, (addr << 1) | 1, iaddr, isize, p_data, size, 1000) == HAL_OK) ? IR_COMPLETE : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Receive(&hi2c1, (addr << 1) | 1, p_data, size, 1000) == HAL_OK) ? IR_COMPLETE : IR_ERROR;
	}
}

static PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Write(&hi2c1, addr << 1, iaddr, isize, p_data, size, 1000) == HAL_OK) ? IR_COMPLETE : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Transmit(&hi2c1, addr << 1, p_data, size, 1000) == HAL_OK) ? IR_COMPLETE : IR_ERROR;
	}
}

#endif

#ifdef USE_INTERRUPT

static PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Read_IT(&hi2c1, (addr << 1) | 1, iaddr, isize, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Receive_IT(&hi2c1, (addr << 1) | 1, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
}

static PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Write_IT(&hi2c1, addr << 1, iaddr, isize, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Transmit_IT(&hi2c1, addr << 1, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
}

#endif

#ifdef USE_DMA

static PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Read_DMA(&hi2c1, (addr << 1) | 1, iaddr, isize, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Receive_DMA(&hi2c1, (addr << 1) | 1, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
}

static PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	if (isize) {
		return (HAL_I2C_Mem_Write_DMA(&hi2c1, addr << 1, iaddr, isize, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
	else {
		return (HAL_I2C_Master_Transmit_DMA(&hi2c1, addr << 1, p_data, size) == HAL_OK) ? IR_WAIT : IR_ERROR;
	}
}

#endif

#if defined(USE_INTERRUPT) || defined(USE_DMA)

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C1) {
		pifI2cPort_sigEndTransfer(&g_i2c_port, TRUE);
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C1) {
		pifI2cPort_sigEndTransfer(&g_i2c_port, TRUE);
	}
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C1) {
		pifI2cPort_sigEndTransfer(&g_i2c_port, TRUE);
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C1) {
		pifI2cPort_sigEndTransfer(&g_i2c_port, TRUE);
	}
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  pif_Init(NULL);

  if (!pifTaskManager_Init(TASK_SIZE)) return -1;

  if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return -1;			// 1000us

  if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return -1;
  if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return -1;			// 1ms
  if (!pifUart_AllocTxBuffer(&s_uart_log, 256)) return -1;
  s_uart_log.act_start_transfer = actLogStartTransfer;

  pifLog_Init();
  if (!pifLog_AttachUart(&s_uart_log)) return -1;

  if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return -1;

  if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 3, 16)) return -1;
  g_i2c_port.act_read = actI2cRead;
  g_i2c_port.act_write = actI2cWrite;

  if (!appSetup()) return -1;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***             ex_gy86              ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
