// Do not remove the include below
#include "exGpsSerialNmea.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LOG_RX				2
#define PIN_LOG_TX				3
#define PIN_GPS_RX				4

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

#define UART_LOG_BAUDRATE		115200
#define UART_GPS_BAUDRATE		9600


static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static uint16_t actLogSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static uint16_t actGpsReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
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

static void _actGpioWrite(uint16_t port, SWITCH state)
{
	static BOOL sw[3];

	if (state) {
		switch (port) {
		case PIF_ID_USER(0):
			digitalWrite(PIN_LOG_TX, sw[1]);
			sw[1] ^= 1;
			break;
		}
	}
	else {
		switch (port) {
		case PIF_ID_USER(0):
			digitalWrite(PIN_LOG_RX, sw[0]);
			sw[0] ^= 1;
			break;

		case PIF_ID_USER(1):
			digitalWrite(PIN_GPS_RX, sw[2]);
			sw[2] ^= 1;
			break;
		}
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
	pinMode(PIN_LOG_RX, OUTPUT);
	pinMode(PIN_LOG_TX, OUTPUT);
	pinMode(PIN_GPS_RX, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE);
	Serial1.begin(UART_GPS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

    pif_act_gpio_write = _actGpioWrite;

	if (!pifUart_Init(&s_uart_log, PIF_ID_USER(0), UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, TM_EXTERNAL_ORDER, 0, "UartTxLog")) return;
    if (!pifUart_AttachRxTask(&s_uart_log, TM_PERIOD, 200000, "UartRxLog")) return;			// 200ms
	s_uart_log.act_receive_data = actLogReceiveData;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 512)) return;										// 512bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_USER(1), UART_GPS_BAUDRATE)) return;
    if (!pifUart_AttachRxTask(&g_uart_gps, TM_PERIOD, 200000, "UartRxGPS")) return;			// 200ms
    g_uart_gps.act_receive_data = actGpsReceiveData;

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***          exGpsSerialNmea         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
