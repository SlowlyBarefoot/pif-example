// Do not remove the include below
#include "ex_gps_serial_ublox.h"
#include "app_main.h"


#define PIN_LED_L				13

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2

#define UART_LOG_BAUDRATE		115200
#define UART_GPS_BAUDRATE			9600


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

static uint16_t actLogReceiveData(PifUart *p_owner, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_owner;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static BOOL actGpsSetBaudrate(PifUart *p_owner, uint32_t baudrate)
{
	Serial2.end();
	pifUart_AbortRx(p_owner);
	Serial2.begin(baudrate);
	return TRUE;
}

static uint16_t actGpsSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial2.write((char *)pucBuffer, usSize);
}

static uint16_t actGpsReceiveData(PifUart *p_owner, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_owner;

	for (i = 0; i < size; i++) {
		data = Serial2.read();
		if (data < 0) break;
		p_data[i] = data;
		if (g_print_data == 2) Serial.write(data);
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

	Serial.begin(UART_LOG_BAUDRATE);
	Serial2.begin(UART_GPS_BAUDRATE);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 10, "UartLog")) return;			// 10ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO, UART_GPS_BAUDRATE)) return;
    if (!pifUart_AttachTask(&g_uart_gps, TM_PERIOD_MS, 10, "UartGPS")) return;			// 10ms
    g_uart_gps.act_receive_data = actGpsReceiveData;
    g_uart_gps.act_send_data = actGpsSendData;
    g_uart_gps.act_set_baudrate = actGpsSetBaudrate;

//    if (!appSetup(9600)) return;
    if (!appSetup(57600)) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
