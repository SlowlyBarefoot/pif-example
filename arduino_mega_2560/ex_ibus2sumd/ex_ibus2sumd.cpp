// Do not remove the include below
#include "ex_ibus2sumd.h"
#include "app_main.h"

#include "core/pif_log.h"

#include <MsTimer2.h>


#define PIN_LED_L				13


uint16_t actLogSendData(PifUart* p_owner, uint8_t* p_buffer, uint16_t size)
{
	(void)p_owner;

    return Serial.write((char *)p_buffer, size);
}

void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

BOOL actSerial1ReceiveData(PifUart* p_owner, uint8_t* p_data)
{
	int data;

	(void)p_owner;

	data = Serial1.read();
	if (data >= 0) {
		*p_data = data;
		return TRUE;
	}
	return FALSE;
}

uint16_t actSerial2SendData(PifUart* p_owner, uint8_t* p_buffer, uint16_t size)
{
	(void)p_owner;

	return Serial2.write((char *)p_buffer, size);
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

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);
	Serial1.begin(115200);
	Serial2.begin(115200);

    appSetup(NULL);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
