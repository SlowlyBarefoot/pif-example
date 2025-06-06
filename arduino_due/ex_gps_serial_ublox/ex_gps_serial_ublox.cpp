// Do not remove the include below
#include "ex_gps_serial_ublox.h"
#include "app_main.h"


#define PIN_LED_L				13

#define TASK_SIZE				8
#define TIMER_1MS_SIZE			2

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
	Serial1.end();
	pifUart_AbortRx(p_owner);
	Serial1.begin(baudrate);
	return TRUE;
}

static uint16_t actGpsSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial1.write((char *)pucBuffer, usSize);
}

static uint16_t actGpsReceiveData(PifUart *p_owner, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_owner;

	for (i = 0; i < size; i++) {
		data = Serial1.read();
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
	Serial1.begin(UART_GPS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 200000, "UartRxLog")) return;	// 200ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return;												// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO, UART_GPS_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&g_uart_gps, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxGPS")) return;
    if (!pifUart_AttachRxTask(&g_uart_gps, PIF_ID_AUTO, TM_PERIOD, 10000, "UartRxGPS")) return;		// 10ms
    g_uart_gps.act_receive_data = actGpsReceiveData;
    g_uart_gps.act_send_data = actGpsSendData;
    g_uart_gps.act_set_baudrate = actGpsSetBaudrate;

    if (!appSetup(57600)) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***       ex_gps_serial_ublox        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
