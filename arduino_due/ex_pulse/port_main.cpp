#include "ex_pulse.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_PULSE				37

#define UART_LOG_BAUDRATE		115200


static uint32_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = FALSE;

	(void)p_task;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
    return 0;
}

static uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void _isrPulse()
{
	pifPulse_sigEdge(&g_pulse, digitalRead(PIN_PULSE) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
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
	pinMode(PIN_PULSE, INPUT_PULLUP);

	attachInterrupt(PIN_PULSE, _isrPulse, CHANGE);

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100000, _taskLedToggle, NULL, TRUE)) return FALSE;	// 100ms
	return TRUE;
}
