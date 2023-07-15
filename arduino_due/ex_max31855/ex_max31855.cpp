// Do not remove the include below
#include <SPI.h>

#include "ex_max31855.h"
#include "app_main.h"


#define PIN_LED_L				13


uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
}

BOOL actTransfer(PifId id, uint8_t* p_write, uint8_t* p_read, uint16_t size)
{
	uint16_t i;

	(void)id;

	digitalWrite(SS, LOW);
	if (p_write) {
		if (p_read) {
			for (i = 0; i < size; i++) {
				p_read[i] = SPI.transfer(p_write[i]);
			}
		}
		else {
			for (i = 0; i < size; i++) {
				SPI.transfer(p_write[i]);
			}
		}
	}
	else {
		if (p_read) {
			for (i = 0; i < size; i++) {
				p_read[i] = SPI.transfer(0);
			}
		}
	}
	digitalWrite(SS, HIGH);
	return TRUE;
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

	SPI.begin();
//	SPI.setClockDivider(SPI_CLOCK_DIV16);
	digitalWrite(SS, HIGH);

	Serial.begin(115200);

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
