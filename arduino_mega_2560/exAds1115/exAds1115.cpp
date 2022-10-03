// Do not remove the include below
#include "exAds1115.h"
#include "appMain.h"

#include "core/pif_log.h"

#include <MsTimer2.h>
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
	uint8_t error;
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
    error = Wire.endTransmission();
    if (error != 0) goto fail;
#else
	if (!I2C_WriteAddr(addr, iaddr, isize, p_data, size)) {
		error = pif_error;
		goto fail;
	}
#endif
    return IR_COMPLETE;

fail:
	pifLog_Printf(LT_ERROR, "I2CW(%Xh): C=%Xh, S=%u E=%u", p_data[0], size, error);
	return IR_ERROR;
}

PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	uint8_t error;
#ifdef USE_I2C_WIRE
	int i;
	uint8_t count;

	if (isize > 0) {
		Wire.beginTransmission(addr);
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	    error = Wire.endTransmission();
	    if (error != 0) goto fail;
	}

    count = Wire.requestFrom(addr, (uint8_t)size);
    if (count < size) goto fail;

    for (i = 0; i < size; i++) {
    	p_data[i] = Wire.read();
    }
#else
	if (!I2C_ReadAddr(addr, iaddr, isize, p_data, size)) {
		error = pif_error;
		goto fail;
	}
#endif
    return IR_COMPLETE;

fail:
	pifLog_Printf(LT_ERROR, "I2CR(%Xh): C=%Xh S=%u E=%u", p_data[0], size, error);
	return IR_ERROR;
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

#ifdef USE_I2C_WIRE
	Wire.begin();
#else
	I2C_Init(I2C_CLOCK_400KHz);
#endif

    //appSetup(NULL);
    appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
