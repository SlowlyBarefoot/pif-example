// Do not remove the include below
#include "ex_pulse.h"
#include "app_main.h"


#define PIN_LED_L				13

#define PIN_PULSE				37

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200


static uint16_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = LOW;

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

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PULSE, INPUT_PULLUP);

	attachInterrupt(PIN_PULSE, _isrPulse, CHANGE);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;			// 100ms

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
