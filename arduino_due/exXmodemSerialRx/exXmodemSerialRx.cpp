// Do not remove the include below
#include "exXmodemSerialRx.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH			29

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200
#define UART_SERIAL_BAUDRATE	115200

//#define USE_SERIAL_USB		// Linux or Windows
#define USE_SERIAL_3			// Other Anduino


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static uint16_t actPushSwitchAcquire(PifSensor* p_owner)
{
	(void)p_owner;

	return !digitalRead(PIN_PUSH_SWITCH);
}

static uint16_t actXmodemSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

#ifdef USE_SERIAL_USB
	return SerialUSB.write((char *)pucBuffer, usSize);
#endif
#ifdef USE_SERIAL_3
   	return Serial3.write((char *)pucBuffer, usSize);
#endif
}

static uint16_t actXmodemReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

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
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE);
#ifdef USE_SERIAL_USB
	SerialUSB.begin(UART_SERIAL_BAUDRATE);
#endif
#ifdef USE_SERIAL_3
	Serial3.begin(UART_SERIAL_BAUDRATE);
#endif

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) return;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) return;

	if (!pifUart_Init(&g_serial, PIF_ID_AUTO, UART_SERIAL_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&g_serial, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxSerial")) return;
    if (!pifUart_AttachRxTask(&g_serial, PIF_ID_AUTO, TM_PERIOD, 50000, "UartRxSerial")) return;	// 50ms
    g_serial.act_receive_data = actXmodemReceiveData;
    g_serial.act_send_data = actXmodemSendData;

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exXmodemSerialRx         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
