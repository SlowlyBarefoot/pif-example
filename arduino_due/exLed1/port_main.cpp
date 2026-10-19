#include "exLed1.h"
#include "app_port.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
	digitalWrite(PIN_LED_GREEN, (unState >> 2) & 1);
	pifLog_Printf(LT_INFO, "RGB:%u S:%x", __LINE__, unState);
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
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;										// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) goto fail;

    if (!pifLed_Init(&g_led_rgb, PIF_ID_AUTO, 3, actLedRGBState)) goto fail;
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
