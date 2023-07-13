// Do not remove the include below
#include "ex_basic_xmodem.h"
#include "linker.h"


#define PIN_LED_L				13


void actLedLState(PifId pif_id, uint32_t state)
{
	(void)pif_id;

	digitalWrite(PIN_LED_L, state & 1);
}

uint16_t actLogSendData(PifComm *p_owner, uint8_t *p_buffer, uint16_t size)
{
	(void)p_owner;

    return Serial.write((char *)p_buffer, size);
}

BOOL actLogReceiveData(PifComm *p_owner, uint8_t *p_data)
{
	int data;

	(void)p_owner;

	data = Serial.read();
	if (data >= 0) {
		*p_data = data;
		return TRUE;
	}
	return FALSE;
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

	Serial.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
