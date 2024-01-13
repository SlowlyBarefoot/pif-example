// Do not remove the include below
#include <DueTimer.h>

#include "exServoMotor.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_PWM					9

#define TASK_SIZE				4
#define TIMER_1MS_SIZE			1
#define TIMER_100US_SIZE		1

#define UART_LOG_BAUDRATE		115200


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

static void actPulsePwm(SWITCH swValue)
{
	digitalWrite(PIN_PWM, swValue);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

static void timer_100us()
{
	pifTimerManager_sigTick(&g_timer_100us);
}

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PWM, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us
    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, TIMER_100US_SIZE)) return;	// 100us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;

    g_pstPwm = pifTimerManager_Add(&g_timer_100us, TT_PWM);
    if (!g_pstPwm) return;
    g_pstPwm->act_pwm = actPulsePwm;

	if (!appSetup()) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer1ms=%d/%d Timer100us=%d\n", pifTaskManager_Count(), TASK_SIZE,
			pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE, pifTimerManager_Count(&g_timer_100us), TIMER_100US_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
