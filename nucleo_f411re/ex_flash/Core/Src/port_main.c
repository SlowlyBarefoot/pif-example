#include "main.h"
#include "app_port.h"


#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_6   /* Start @ of user Flash area */

#define FLASH_RECORD_SIZE		131072		// 128kbytes, one sector per record


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

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = OFF;

	(void)pvIssuer;

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sw);
	sw ^= 1;
}

static uint32_t GetSector(uint32_t address)
{
	uint32_t sector = 0;

	if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0)) {
		sector = FLASH_SECTOR_0;	// 16Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1)) {
		sector = FLASH_SECTOR_1;	// 16Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2)) {
		sector = FLASH_SECTOR_2;	// 16Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3)) {
		sector = FLASH_SECTOR_3;	// 16Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4)) {
		sector = FLASH_SECTOR_4;	// 64Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5)) {
		sector = FLASH_SECTOR_5;	// 128Kbytes
	}
	else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6)) {
		sector = FLASH_SECTOR_6;	// 128Kbytes
	}
	else {
		sector = FLASH_SECTOR_7;
	}

	return sector;
}

// Erases the sector that begins at the address. pifFlash calls it when the record reaches the sector,
// with the controller already unlocked.
static BOOL actFlashErase(PifFlash* p_owner, uint32_t addr)
{
	FLASH_EraseInitTypeDef erase;
	uint32_t sector_error = 0;

	(void)p_owner;

	erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	erase.Sector = GetSector(addr);
	erase.NbSectors = 1;
	if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK) return FALSE;

	// The caches may still hold what the sector read as before the erase.
	__HAL_FLASH_DATA_CACHE_DISABLE();
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

	__HAL_FLASH_DATA_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();

	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	__HAL_FLASH_DATA_CACHE_ENABLE();
	return TRUE;
}

// Programs one word, the program unit of the F4 at 2.7 to 3.6V.
static BOOL actFlashProgram(PifFlash* p_owner, uint32_t addr, const uint8_t* p_data, uint16_t size)
{
	(void)p_owner;
	(void)size;

	return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(const uint32_t*)p_data) == HAL_OK;
}

static BOOL actFlashRead(PifFlash* p_owner, uint32_t addr, uint8_t* p_data, size_t size)
{
	(void)p_owner;

	memcpy(p_data, (const void*)addr, size);
	return TRUE;
}

static void actFlashLock(PifFlash* p_owner, BOOL lock)
{
	(void)p_owner;

	if (lock) HAL_FLASH_Lock();
	else HAL_FLASH_Unlock();
}

static BOOL initFlash(void)
{
	int i;

	for (i = 0; i < FLASH_RECORD_COUNT; i++) {
		if (!pifFlash_Init(&g_flash[i], PIF_ID_AUTO, FLASH_USER_START_ADDR + i * FLASH_RECORD_SIZE, FLASH_RECORD_SIZE,
				FLASH_RECORD_SIZE, 4, actFlashErase, actFlashProgram, actFlashRead)) return FALSE;
		pifFlash_AttachActLock(&g_flash[i], actFlashLock);
	}
	return TRUE;
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, huart2.Init.BaudRate)) return FALSE;
	if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return FALSE;		// 200ms
	if (!pifUart_AllocRxBuffer(&s_uart_log, 64)) return FALSE;										// 64bytes
	if (!pifUart_AllocTxBuffer(&s_uart_log, 128)) return FALSE;										// 128bytes
	s_uart_log.act_start_transfer = actLogStartTransfer;

	HAL_UART_Receive_IT(&huart2, &s_ucLogRx, 1);

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;											// 256bytes

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
	if (!g_timer_led) return FALSE;
	pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);

	if (!initFlash()) return FALSE;
	return TRUE;
}
