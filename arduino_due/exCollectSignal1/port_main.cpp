#include "exCollectSignal1.h"
#include "app_port.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_BLUE			27

#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31
#define PIN_PUSH_SWITCH_3		33

#define UART_LOG_BAUDRATE		115200


static struct {
	uint8_t pin_switch;
	uint8_t pin_led;
} s_sequence_test[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


static void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static void actGpioRGBState(PifId pif_id, uint8_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_RED, state & 1);
	digitalWrite(PIN_LED_YELLOW, (state >> 1) & 1);
}

static void actLedCollectState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_BLUE, state & 1);
}

static int32_t actPushSwitchAcquire(PifSensor *p_owner)
{
	return !digitalRead(s_sequence_test[p_owner->_id - PIF_ID_SWITCH].pin_switch);
}

static int32_t actPushSwitchCollectAcquire(PifSensor *p_owner)
{
	(void)p_owner;

	return !digitalRead(PIN_PUSH_SWITCH_3);
}

static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_3, INPUT_PULLUP);

	Serial.begin(UART_LOG_BAUDRATE);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 1024)) return FALSE;										// 1024bytes

    if (!pifCollectSignal_Init("example", NULL, 0)) return FALSE;
    if (!pifCollectSignal_ChangeScale(CSS_1MS)) return FALSE;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

    if (!pifGpio_Init(&g_gpio_rgb, PIF_ID_AUTO, SEQUENCE_COUNT)) return FALSE;
    pifGpio_AttachActOut(&g_gpio_rgb, actGpioRGBState);

    if (!pifLed_Init(&g_led_collect, PIF_ID_AUTO, 1, actLedCollectState)) return FALSE;

    for (int i = 0; i < SEQUENCE_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_test[i].push_switch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) return FALSE;
    }

    if (!pifSensorSwitch_Init(&g_push_switch_collect, PIF_ID_AUTO, 0, actPushSwitchCollectAcquire)) return FALSE;
	return TRUE;
}
