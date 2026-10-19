#include "exLogPrint.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void evtLedToggle(void *pvIssuer)
{
	static BOOL sw = LOW;

	(void)pvIssuer;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	pifLog_Printf(LT_INFO, "LED: %u", sw);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 80)) goto fail;										// 80bytes

    g_timer_led = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_led) goto fail;
    pifTimer_AttachEvtFinish(g_timer_led, evtLedToggle, NULL, FALSE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
