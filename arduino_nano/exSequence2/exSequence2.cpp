// Do not remove the include below
#include <MsTimer2.h>

#include "exSequence2.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3

#define PIN_PUSH_SWITCH_1		5
#define PIN_PUSH_SWITCH_2		6

#define TASK_SIZE				6
#define TIMER_1MS_SIZE			4


static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


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

static void actLedRGBState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(s_stSequenceTest[0].ucPinLed, unState & 1);
	digitalWrite(s_stSequenceTest[1].ucPinLed, (unState >> 1) & 1);
}

static uint16_t actPushSwitchAcquire(PifSensor* p_owner)
{
	return !digitalRead(s_stSequenceTest[p_owner->_id - PIF_ID_SWITCH].ucPinSwitch);
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    if (!pifLed_Init(&g_led_rgb, PIF_ID_AUTO, &g_timer_1ms, SEQUENCE_COUNT, actLedRGBState)) return;

    for (int i = 0; i < SEQUENCE_COUNT; i++) {
		if (!pifSensorSwitch_Init(&g_stSequenceTest[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire)) return;
    }

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
