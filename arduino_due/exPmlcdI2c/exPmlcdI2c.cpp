// Do not remove the include below
#include <Wire.h>

#include "exPmlcdI2c.h"
#include "appMain.h"

#include "pif_log.h"


#define PIN_LED_L				13


uint16_t actLogSendData(PifComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

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
    uint8_t status_ = Wire.endTransmission();
    if (status_ != 0) {
    	pifLog_Printf(LT_ERROR, "I2CW(%Xh): C=%Xh S=%u E=%u", addr, p_data[0], size, status_);
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
    pif_Loop();
}
