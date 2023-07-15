// Do not remove the include below
#include "exPmlcdI2c.h"
#include "appMain.h"

#include "core/pif_log.h"

#include <Wire.h>


#define PIN_LED_L				13


uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	uint16_t i;

	(void)iaddr;
	(void)isize;

	Wire.beginTransmission(addr);
    for (i = 0; i < size; i++) {
    	Wire.write(p_data[i]);
    }
    if (Wire.endTransmission() != 0) {
		pif_error = E_TRANSFER_FAILED;
        return IR_ERROR;
    }
    return IR_COMPLETE;
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

	Wire.begin();

    appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
