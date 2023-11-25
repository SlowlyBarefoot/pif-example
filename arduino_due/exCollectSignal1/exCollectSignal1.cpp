// Do not remove the include below
#include "exCollectSignal1.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_BLUE			27

#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31
#define PIN_PUSH_SWITCH_3		33

#define TASK_SIZE				7
#define TIMER_1MS_SIZE			4


static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

static void actGpioRGBState(PifId usPifId, uint8_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_RED, unState & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 1) & 1);
}

static void actLedCollectState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_BLUE, unState & 1);
}

static uint16_t actPushSwitchAcquire(PifSensor* p_owner)
{
	return !digitalRead(s_stSequenceTest[p_owner->_id - PIF_ID_SWITCH].ucPinSwitch);
}

static uint16_t actPushSwitchCollectAcquire(PifSensor* p_owner)
{
	(void)p_owner;

	return !digitalRead(PIN_PUSH_SWITCH_3);
}

static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size, uint8_t* p_rate)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	if (p_rate) *p_rate = 100 * Serial.available() / SERIAL_BUFFER_SIZE;
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
	PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_3, INPUT_PULLUP);

	Serial.begin(115200);

	pif_Init(micros);

	if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    pifCollectSignal_Init("example");
    if (!pifCollectSignal_ChangeScale(CSS_1MS)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!pifGpio_Init(&g_gpio_rgb, PIF_ID_AUTO, SEQUENCE_COUNT)) return;
    pifGpio_AttachActOut(&g_gpio_rgb, actGpioRGBState);

    if (!pifLed_Init(&g_led_collect, PIF_ID_AUTO, &g_timer_1ms, 1, actLedCollectState)) return;

    for (int i = 0; i < SEQUENCE_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_test[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) return;
    }

    if (!pifSensorSwitch_Init(&g_push_switch_collect, PIF_ID_AUTO, 0, actPushSwitchCollectAcquire)) return;

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
