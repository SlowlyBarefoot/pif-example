// Do not remove the include below
#include <MsTimer2.h>

#include "exSwitch3.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3

#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			3

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actLedState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	digitalWrite(PIN_LED_RED, (unState >> 1) & 1);
	digitalWrite(PIN_LED_YELLOW, (unState >> 2) & 1);
}

static void evtSwitchAcquire(void *pvIssuer)
{
	(void)pvIssuer;

	pifSensorSwitch_sigData(&g_push_switch, !digitalRead(PIN_PUSH_SWITCH));
	pifSensorSwitch_sigData(&g_tilt_switch, digitalRead(PIN_TILT_SWITCH));
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
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTxTask(&s_uart_log, TM_EXTERNAL_ORDER, 0, NULL)) return;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 80)) return;										// 80bytes

    if (!pifLed_Init(&g_led, PIF_ID_AUTO, &g_timer_1ms, 3, actLedState)) return;

    g_timer_switch = pifTimerManager_Add(&g_timer_1ms, TT_REPEAT);
    if (!g_timer_switch) return;
    pifTimer_AttachEvtFinish(g_timer_switch, evtSwitchAcquire, NULL);

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            exSwitch3             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
    pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
