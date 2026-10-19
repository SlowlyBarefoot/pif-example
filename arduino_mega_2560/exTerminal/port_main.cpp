#include "exTerminal.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200


static PifUart s_uart_log;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((const char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *pucData, uint16_t size)
{
	int rxData;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		rxData = Serial.read();
		if (rxData < 0) break;
		pucData[i] = rxData;
	}
	return i;
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	static uint32_t state;

	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	if (unState != state) {
		pifLog_Printf(LT_INFO, "LED State=%u", unState);
		state = unState;
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
BOOL portSetup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) goto fail;	// 200ms
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;										// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) goto fail;
    return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
