// Do not remove the include below
#include "ex_hc_sr04.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_TRIGGER 			23
#define PIN_ECHO				22

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

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
	static BOOL swLed = OFF;

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

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_TRIGGER, OUTPUT);
	pinMode(PIN_ECHO, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    attachInterrupt(PIN_ECHO, _isrUltrasonicEcho, CHANGE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, TM_EXTERNAL_ORDER, 0, NULL)) return;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;										// 256bytes

    if (!pifHcSr04_Init(&g_hcsr04, PIF_ID_AUTO)) return;
	g_hcsr04.act_trigger = actHcSr04Trigger;

    if (!pifTaskManager_Add(TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return;			// 500ms

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_hc_sr04            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
