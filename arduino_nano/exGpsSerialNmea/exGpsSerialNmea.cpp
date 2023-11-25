// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exGpsSerialNmea.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH			2

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1


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

static uint16_t actGpsSendData(PifUart *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return serialGps.write((char *)pucBuffer, usSize);
}

static uint16_t actGpsReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size, uint8_t* p_rate)
{
	int data;
	uint16_t i;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = serialGps.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * serialGps.available() / SERIAL_RX_BUFFER_SIZE;
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

	Serial.begin(115200);
	serialGps.begin(9600);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) return;				// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;

	if (!pifSensorSwitch_Init(&g_push_switch, PIF_ID_AUTO, 0, actPushSwitchAcquire)) return;

	if (!pifUart_Init(&g_uart_gps, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart_gps, TM_PERIOD_MS, 1, "UartGPS")) return;				// 1ms
    g_uart_gps.act_receive_data = actGpsReceiveData;
    g_uart_gps.act_send_data = actGpsSendData;

    if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
