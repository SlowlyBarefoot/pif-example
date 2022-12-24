// Do not remove the include below
#include "ex_rc_ppm.h"
#include "app_main.h"


#define PIN_LED_L				13
#define PIN_PULSE				37


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
	pifRcPpm_sigTick(&g_rc_ppm, micros());
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

	attachInterrupt(PIN_PULSE, _isrPulse, RISING);

	Serial.begin(115200);

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
