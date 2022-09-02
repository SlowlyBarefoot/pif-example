// Do not remove the include below
#include "ex_pulse.h"
#include "app_main.h"


#define PIN_LED_L				13
#define PIN_PULSE				30


void actLedL(SWITCH sw)
{
	digitalWrite(PIN_LED_L, sw);
}

uint16_t actLogSendData(PifComm* p_comm, uint8_t* p_buffer, uint16_t size)
{
	(void)p_comm;

    return Serial.write((char *)p_buffer, size);
}

static void _isrPulse()
{
	pifPulse_sigEdge(&g_pulse, digitalRead(PIN_PULSE) ? PE_RISING : PE_FALLING, micros());
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
	pinMode(PIN_PULSE, INPUT_PULLUP);

	attachInterrupt(PIN_PULSE, _isrPulse, CHANGE);

	Serial.begin(115200);

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
