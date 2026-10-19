#include "ex_basic_xmodem.h"
#include "app_port.h"


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200


static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint16_t actLogSendData(PifUart* p_owner, uint8_t* p_buffer, uint16_t size)
{
	(void)p_owner;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;		// 1000us

	if (!pifUart_Init(&g_uart, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&g_uart, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) goto fail;
    if (!pifUart_AttachRxTask(&g_uart, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) goto fail;	// 200ms
	g_uart.act_receive_data = actLogReceiveData;
	g_uart.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&g_uart, 256)) goto fail;												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) goto fail;
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
