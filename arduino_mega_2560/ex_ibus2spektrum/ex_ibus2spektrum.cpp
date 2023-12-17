// Do not remove the include below
#include "ex_ibus2spektrum.h"
#include "app_main.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1


static uint16_t actLogSendData(PifUart* p_owner, uint8_t* p_buffer, uint16_t size)
{
	(void)p_owner;

    return Serial.write((char *)p_buffer, size);
}

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint16_t actSerial1ReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial1.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static uint16_t actSerial2SendData(PifUart* p_owner, uint8_t* p_buffer, uint16_t size)
{
	(void)p_owner;

	return Serial2.write((char *)p_buffer, size);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);
	Serial1.begin(115200);
	Serial2.begin(115200);

    pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

	if (!pifUart_Init(&g_uart_ibus, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_ibus, TM_PERIOD_MS, 1, "UartIbus")) return;					// 1ms
	g_uart_ibus.act_receive_data = actSerial1ReceiveData;

	if (!pifUart_Init(&g_uart_spektrum, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_spektrum, TM_PERIOD_MS, 1, "UartSpektrum")) return;			// 1ms
    g_uart_spektrum.act_send_data = actSerial2SendData;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
