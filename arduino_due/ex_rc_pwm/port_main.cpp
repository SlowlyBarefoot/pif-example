#include "ex_rc_pwm.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_RC_PWM_1			37
#define PIN_RC_PWM_2			38
#define PIN_RC_PWM_3			39
#define PIN_RC_PWM_4			40
#define PIN_RC_PWM_5			41
#define PIN_RC_PWM_6			42

#define UART_LOG_BAUDRATE		115200


static uint32_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = FALSE;

	(void)p_task;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
    return 0;
}

static uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void _isrRcPwm1()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 0, digitalRead(PIN_RC_PWM_1) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
}

static void _isrRcPwm2()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 1, digitalRead(PIN_RC_PWM_2) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
}

static void _isrRcPwm3()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 2, digitalRead(PIN_RC_PWM_3) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
}

static void _isrRcPwm4()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 3, digitalRead(PIN_RC_PWM_4) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
}

static void _isrRcPwm5()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 4, digitalRead(PIN_RC_PWM_5) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
}

static void _isrRcPwm6()
{
	pifRcPwm_sigEdge(&g_rc_pwm, 5, digitalRead(PIN_RC_PWM_6) ? PS_RISING_EDGE : PS_FALLING_EDGE, micros());
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
	pinMode(PIN_RC_PWM_1, INPUT_PULLUP);
	pinMode(PIN_RC_PWM_2, INPUT_PULLUP);
	pinMode(PIN_RC_PWM_3, INPUT_PULLUP);
	pinMode(PIN_RC_PWM_4, INPUT_PULLUP);
	pinMode(PIN_RC_PWM_5, INPUT_PULLUP);
	pinMode(PIN_RC_PWM_6, INPUT_PULLUP);

	attachInterrupt(PIN_RC_PWM_1, _isrRcPwm1, CHANGE);
	attachInterrupt(PIN_RC_PWM_2, _isrRcPwm2, CHANGE);
	attachInterrupt(PIN_RC_PWM_3, _isrRcPwm3, CHANGE);
	attachInterrupt(PIN_RC_PWM_4, _isrRcPwm4, CHANGE);
	attachInterrupt(PIN_RC_PWM_5, _isrRcPwm5, CHANGE);
	attachInterrupt(PIN_RC_PWM_6, _isrRcPwm6, CHANGE);

	Serial.begin(UART_LOG_BAUDRATE);

	pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

	pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100000, _taskLedToggle, NULL, TRUE)) return FALSE;	// 100ms
	return TRUE;
}
