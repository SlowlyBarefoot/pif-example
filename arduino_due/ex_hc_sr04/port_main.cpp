#include "ex_hc_sr04.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_TRIGGER 			23
#define PIN_ECHO				22

#define UART_LOG_BAUDRATE		115200


static SWITCH s_echo_state;


static uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void actHcSr04Trigger(SWITCH state)
{
    digitalWrite(PIN_TRIGGER, state);
    s_echo_state = 0;
}

static void _isrUltrasonicEcho()
{
	s_echo_state ^= 1;
	pifHcSr04_sigReceiveEcho(&g_hcsr04, s_echo_state);
}

static uint32_t taskLedToggle(PifTask* p_task)
{
	static BOOL swLed = FALSE;

	(void)p_task;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
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
	pinMode(PIN_TRIGGER, OUTPUT);
	pinMode(PIN_ECHO, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    attachInterrupt(PIN_ECHO, _isrUltrasonicEcho, CHANGE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;												// 256bytes

    if (!pifHcSr04_Init(&g_hcsr04, PIF_ID_AUTO, NULL)) goto fail;
	g_hcsr04.act_trigger = actHcSr04Trigger;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;	// 500ms
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
