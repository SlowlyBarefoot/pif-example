#include "ex_rtc_ds1302.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_LED_L				13

// Wiring of the DS1302 module
#define PIN_DS1302_SCLK			6		// CLK
#define PIN_DS1302_IO			7		// DAT
#define PIN_DS1302_CE			8		// RST

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int rx_data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		rx_data = Serial.read();
		if (rx_data < 0) break;
		p_data[i] = rx_data;
	}
	return i;
}

static void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

static void actDs1302Write(PifDs1302Pin pin, SWITCH state)
{
	static const uint8_t pins[] = { PIN_DS1302_CE, PIN_DS1302_SCLK, PIN_DS1302_IO };

	digitalWrite(pins[pin], state);
}

static SWITCH actDs1302Read()
{
	return digitalRead(PIN_DS1302_IO);
}

static void actDs1302IoMode(BOOL input)
{
	pinMode(PIN_DS1302_IO, input ? INPUT : OUTPUT);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_DS1302_CE, OUTPUT);
	pinMode(PIN_DS1302_SCLK, OUTPUT);
	pinMode(PIN_DS1302_IO, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, NULL)) return FALSE;	// 200ms
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    if (!pifLog_Init()) return FALSE;
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;										// 256bytes

    if (!pifDs1302_Init(&g_ds1302, PIF_ID_AUTO, actDs1302Write, actDs1302Read, actDs1302IoMode)) return FALSE;
    pifDs1302_SetDelay(&g_ds1302, 0);		// digitalWrite() alone takes longer than a half period of SCLK.

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
    return TRUE;
}
