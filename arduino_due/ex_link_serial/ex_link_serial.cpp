// Do not remove the include below
#include "ex_link_serial.h"
#include "app_main.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define TASK_SIZE				6
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200
#define UART_SERIAL_BAUDRATE	115200

#define USE_SERIAL_USB			// Linux or Windows
//#define USE_SERIAL_3			// Other Anduino


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

static uint16_t actPushSwitchAcquire(PifSensor *p_owner)
{
	return !digitalRead(s_pin_switch[p_owner->_id - PIF_ID_SWITCH]);
}

static uint16_t actSerialSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

#if defined(USE_SERIAL_USB)
    return SerialUSB.write((char *)p_buffer, size);
#elif defined(USE_SERIAL_3)
    return Serial3.write((char *)p_buffer, size);
#else
	#error "Please select a serial port"
#endif
}

static uint16_t actSerialReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
#ifdef USE_SERIAL_USB
		data = SerialUSB.read();
#endif
#ifdef USE_SERIAL_3
		data = Serial3.read();
#endif
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

//The setup function is called once at startup of the sketch
void setup()
{
	int line;
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE);
#if defined(USE_SERIAL_USB)
	SerialUSB.begin(UART_SERIAL_BAUDRATE);
#elif defined(USE_SERIAL_3)
	Serial3.begin(UART_SERIAL_BAUDRATE);
#else
	#error "Please select a serial port"
#endif

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) {
		line = __LINE__;
		goto fail;
	}

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) {			// 1000us
		line = __LINE__;
		goto fail;
	}

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) {
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) {
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxLog")) {	// 1ms
		line = __LINE__;
		goto fail;
	}
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) {
		line = __LINE__;
		goto fail;
	}

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) {
		line = __LINE__;
		goto fail;
	}

    for (int i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifSensorSwitch_Init(&g_link_test[i].push_switch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) {
			line = __LINE__;
			goto fail;
		}
    }

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) {
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachTxTask(&g_serial, PIF_ID_AUTO, TM_PERIOD, 1000, "UartTxSerial")) {		// 1ms
		line = __LINE__;
		goto fail;
	}
    if (!pifUart_AttachRxTask(&g_serial, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxSerial")) {	// 1ms
		line = __LINE__;
		goto fail;
	}
    g_serial.act_receive_data = actSerialReceiveData;
    g_serial.act_send_data = actSerialSendData;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         ex_link_serial        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");

	if (!appSetup()) {
		line = __LINE__;
		goto fail;
	}

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return;

fail:	
	pifLog_Printf(LT_ERROR, "Error: %d Line=%d\n", pif_error, line);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
