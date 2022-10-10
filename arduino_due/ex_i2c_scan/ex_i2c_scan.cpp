// Do not remove the include below
#include "ex_i2c_scan.h"
#include "app_main.h"

#ifdef USE_I2C_WIRE
	#include <Wire.h>
#else
	#include "../i2c.h"
#endif


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
#ifdef USE_I2C_WIRE
	int i;

	Wire.beginTransmission(addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (i = 0; i < size; i++) {
    	Wire.write(p_data[i]);
    }
    if (Wire.endTransmission() != 0) return IR_ERROR;
#else
	if (!I2C_WriteAddr(addr, iaddr, isize, p_data, size)) {
		return IR_ERROR;
	}
#endif
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

#ifdef USE_I2C_WIRE
	Wire.begin();
#else
	I2C_Init(I2C_CLOCK_400KHz);
#endif

    appSetup(NULL);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
