// Do not remove the include below
#include "exTerminal.h"
#include "appMain.h"


#define PIN_LED_L				13

#define TASK_SIZE				3
#define TIMER_1MS_SIZE			1


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static BOOL actLogReceiveData(PifUart *p_uart, uint8_t *pucData)
{
	int rxData;

	(void)p_uart;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
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

	Serial.begin(115200);

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;				// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!appSetup()) return;

    pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
