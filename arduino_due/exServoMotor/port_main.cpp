#include "exServoMotor.h"
#include "app_port.h"

#include <DueTimer.h>


#define PIN_LED_L				13
#define PIN_PWM					9

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

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PWM, OUTPUT);

	Timer3.attachInterrupt(timer_100us).start(100);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 2)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;		// 1000us
    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, TIMER_100US_SIZE)) return FALSE;	// 100us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;											// 256bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

    if (!pifPwmGen_Init(&g_pwm_gen, PIF_ID_AUTO, &g_timer_100us, actPulsePwm)) return FALSE;
	return TRUE;
}
