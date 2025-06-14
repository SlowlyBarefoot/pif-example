// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exGpsSerialNmea.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH			2

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1

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

static uint16_t actPushSwitchAcquire(PifSensor* p_owner)
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

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);
	serialGps.begin(UART_GPS_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartLog")) return;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 128)) return;											// 128bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) return;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO, UART_GPS_BAUDRATE)) return;
    if (!pifUart_AttachRxTask(&g_uart_gps, PIF_ID_AUTO, TM_PERIOD, 200000, "UartGPS")) return;	// 200ms
    g_uart_gps.act_receive_data = actGpsReceiveData;

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exGpsSerialNmea          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
