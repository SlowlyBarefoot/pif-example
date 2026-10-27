#include "ex_link_serial_loop.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define UART_LOG_BAUDRATE		115200
#define UART_SERIAL_1_BAUDRATE	115200
#define UART_SERIAL_2_BAUDRATE	115200


static uint8_t s_pin_switch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void actLedLState(PifId pid_id, uint32_t state)
{
	(void)pid_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static int32_t actPushSwitchAcquire(PifSensor *p_owner)
{
	return !digitalRead(s_pin_switch[p_owner->_id - PIF_ID_SWITCH]);
}

static uint16_t actSerial1SendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial1.write((char *)p_buffer, size);
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

static uint16_t actSerial2SendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial2.write((char *)p_buffer, size);
}

static uint16_t actSerial2ReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial2.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
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
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_SERIAL_1_BAUDRATE);
	Serial2.begin(UART_SERIAL_2_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxLog")) return FALSE;		// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

    for (int i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifSensorSwitch_Init(&g_link_test[i].push_switch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) return FALSE;
    }

	if (!pifUart_Init(&g_serial1, PIF_ID_AUTO, UART_SERIAL_1_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&g_serial1, PIF_ID_AUTO, TM_PERIOD, 1000, "UartTxSerial1")) return FALSE;		// 1ms
    if (!pifUart_AttachRxTask(&g_serial1, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxSerial1")) return FALSE;		// 1ms
    g_serial1.act_receive_data = actSerial1ReceiveData;
    g_serial1.act_send_data = actSerial1SendData;

	if (!pifUart_Init(&g_serial2, PIF_ID_AUTO, UART_SERIAL_2_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&g_serial2, PIF_ID_AUTO, TM_PERIOD, 1000, "UartTxSerial2")) return FALSE;		// 1ms
    if (!pifUart_AttachRxTask(&g_serial2, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxSerial2")) return FALSE;		// 1ms
    g_serial2.act_receive_data = actSerial2ReceiveData;
    g_serial2.act_send_data = actSerial2SendData;
	return TRUE;
}
