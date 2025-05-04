// Do not remove the include below
#include "ex_ibus2sumd.h"
#include "app_main.h"

#include <MsTimer2.h>


#define PIN_LED_L				13
#define PIN_LOG_TX				2
#define PIN_IBUS_RX				3
#define PIN_SUMD_TX				4

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200
#define UART_IBUS_BAUDRATE		115200
#define UART_SUMD_BAUDRATE		115200


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

static void _actGpioWrite(uint16_t port, SWITCH state)
{
	static BOOL sw[3];

	if (state) {
		switch (port) {
		case PIF_ID_USER(0):
			digitalWrite(PIN_LOG_TX, sw[0]);
			sw[0] ^= 1;
			break;

		case PIF_ID_USER(2):
			digitalWrite(PIN_SUMD_TX, sw[2]);
			sw[2] ^= 1;
			break;
		}
	}
	else {
		switch (port) {
		case PIF_ID_USER(1):
			digitalWrite(PIN_IBUS_RX, sw[1]);
			sw[1] ^= 1;
			break;
		}
	}
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
	pinMode(PIN_LOG_TX, OUTPUT);
	pinMode(PIN_IBUS_RX, OUTPUT);
	pinMode(PIN_SUMD_TX, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_IBUS_BAUDRATE);
	Serial2.begin(UART_SUMD_BAUDRATE);

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    pif_act_gpio_write = _actGpioWrite;

	if (!pifUart_Init(&s_uart_log, PIF_ID_USER(0), UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, TM_EXTERNAL_ORDER, 0, "UartLog")) return;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;										// 256bytes

	if (!pifUart_Init(&g_uart_ibus, PIF_ID_USER(1), UART_IBUS_BAUDRATE)) return;
    if (!pifUart_AttachRxTask(&g_uart_ibus, TM_PERIOD, 3000, "UartIbus")) return;			// 3ms
	g_uart_ibus.act_receive_data = actSerial1ReceiveData;

	if (!pifUart_Init(&g_uart_sumd, PIF_ID_USER(2), UART_SUMD_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&g_uart_sumd, TM_EXTERNAL_ORDER, 0, "UartSumd")) return;
    g_uart_sumd.act_send_data = actSerial2SendData;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           ex_ibus2sumd           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
