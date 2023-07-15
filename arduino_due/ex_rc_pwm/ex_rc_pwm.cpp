// Do not remove the include below
#include "ex_rc_pwm.h"
#include "app_main.h"


#define PIN_LED_L				13
#define PIN_RC_PWM_1			37
#define PIN_RC_PWM_2			38
#define PIN_RC_PWM_3			39
#define PIN_RC_PWM_4			40
#define PIN_RC_PWM_5			41
#define PIN_RC_PWM_6			42


void actLedL(SWITCH sw)
{
	digitalWrite(PIN_LED_L, sw);
}

uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
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

//The setup function is called once at startup of the sketch
void setup()
{
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

	Serial.begin(115200);

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
