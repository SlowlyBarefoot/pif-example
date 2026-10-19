#include "exSwitch3.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actLedState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	digitalWrite(PIN_LED_RED, (unState >> 1) & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 2) & 1);
}

static void evtSwitchAcquire(void *pvIssuer)
{
	(void)pvIssuer;

	pifSensorSwitch_sigData(&g_push_switch, !digitalRead(PIN_PUSH_SWITCH));
	pifSensorSwitch_sigData(&g_tilt_switch, digitalRead(PIN_TILT_SWITCH));
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 80)) goto fail;										// 80bytes

    if (!pifLed_Init(&g_led, PIF_ID_AUTO, 3, actLedState)) goto fail;

    g_timer_switch = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_switch) goto fail;
    pifTimer_AttachEvtFinish(g_timer_switch, evtSwitchAcquire, NULL, FALSE);
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
