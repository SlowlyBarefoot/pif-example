#include "exGpsSerialNmea.h"
#include "app_port.h"

#include <MsTimer2.h>
#include <SoftwareSerial.h>


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH			2

#define UART_LOG_BAUDRATE		115200
#define UART_GPS_BAUDRATE		9600


SoftwareSerial serialGps(8, 9);		// RX, TX


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

static int32_t actPushSwitchAcquire(PifSensor* p_owner)
{
	(void)p_owner;

	return !digitalRead(PIN_PUSH_SWITCH);
}

static uint16_t actGpsReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = serialGps.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);
	serialGps.begin(UART_GPS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) goto fail;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 128)) goto fail;												// 128bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 2, actLedLState)) goto fail;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) goto fail;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO, UART_GPS_BAUDRATE)) goto fail;
    if (!pifUart_AttachRxTask(&g_uart_gps, PIF_ID_AUTO, TM_PERIOD, 200000, "UartGPS")) goto fail;	// 200ms
    g_uart_gps.act_receive_data = actGpsReceiveData;
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
