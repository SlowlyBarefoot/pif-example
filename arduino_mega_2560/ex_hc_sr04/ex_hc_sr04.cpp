// Do not remove the include below
#include <MsTimer2.h>

#include "ex_hc_sr04.h"
#include "appMain.h"


#define PIN_LED_L		13
#define PIN_TRIGGER 	23
#define PIN_ECHO		2


static SWITCH s_echo_state;


uint16_t actLogSendData(PifUart* p_uart, uint8_t* p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

void actHcSr04Trigger(SWITCH state)
{
    digitalWrite(PIN_TRIGGER, state);
    s_echo_state = 0;
}

static void _isrUltrasonicEcho()
{
	s_echo_state ^= 1;
	pifHcSr04_sigReceiveEcho(&g_hcsr04, s_echo_state);
}

uint16_t taskLedToggle(PifTask* p_task)
{
	static BOOL swLed = OFF;

	(void)p_task;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
	pifTimerManager_sigTick(&g_timer_1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_TRIGGER, OUTPUT);
	pinMode(PIN_ECHO, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    attachInterrupt(0, _isrUltrasonicEcho, CHANGE);		// 0 : PIN_ECHO

	appSetup((PifActTimer1us)micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
