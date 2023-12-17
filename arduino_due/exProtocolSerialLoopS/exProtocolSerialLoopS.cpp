// Do not remove the include below
#include "exProtocolSerialLoopS.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define TASK_SIZE				6
#define TIMER_1MS_SIZE			7


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


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
	return !digitalRead(s_ucPinSwitch[p_owner->_id - PIF_ID_SWITCH]);
}

static uint16_t actSerial1SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial1.write((char *)pucBuffer, usSize);
}

static uint16_t actSerial1ReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial1.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

static uint16_t actSerial2SendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial2.write((char *)pucBuffer, usSize);
}

static uint16_t actSerial2ReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

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

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(115200);
	Serial1.begin(115200);
	Serial2.begin(115200);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, "UartLog")) return;				// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    for (int i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifSensorSwitch_Init(&g_stProtocolTest[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) return;
    }

	if (!pifUart_Init(&g_serial1, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_serial1, TM_PERIOD_MS, 1, "UartSerial1")) return;			// 1ms
    g_serial1.act_receive_data = actSerial1ReceiveData;
    g_serial1.act_send_data = actSerial1SendData;

	if (!pifUart_Init(&g_serial2, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_serial2, TM_PERIOD_MS, 1, "UartSerial2")) return;			// 1ms
    g_serial2.act_receive_data = actSerial2ReceiveData;
    g_serial2.act_send_data = actSerial2SendData;

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
