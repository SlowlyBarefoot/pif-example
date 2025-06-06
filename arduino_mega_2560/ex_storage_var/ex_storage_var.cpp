// Do not remove the include below
#include <avr/eeprom.h>
#include <MsTimer2.h>

#include "ex_storage_var.h"
#include "app_main.h"


#define PIN_LED_L				13

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1

#define STORAGE_SECTOR_SIZE		16
#define STORAGE_VOLUME			4096

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static void evtLedToggle(void* p_issuer)
{
	static BOOL sw = OFF;

	(void)p_issuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static BOOL actStorageRead(PifStorage* p_owner, uint8_t* dst, uint32_t src, size_t size)
{
	(void)p_owner;

	eeprom_read_block(dst, (void *)src, size);
	return TRUE;
}

static BOOL actStorageWrite(PifStorage* p_owner, uint32_t dst, uint8_t* src, size_t size)
{
	(void)p_owner;

	eeprom_write_block(src, (void *)dst, size);
	return TRUE;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return;	// 200ms
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return;
	s_uart_log.act_send_data = actLogSendData;
	s_uart_log.act_receive_data = actLogReceiveData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 512)) return;										// 512bytes

	g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) return;
    pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL);

	if (!pifStorageVar_Init(&g_storage, PIF_ID_AUTO)) return;
	if (!pifStorageVar_AttachActStorage(&g_storage, actStorageRead, actStorageWrite)) return;
	if (!pifStorageVar_SetMedia(&g_storage, STORAGE_SECTOR_SIZE, STORAGE_VOLUME, 16)) return;

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          ex_storage_var          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
