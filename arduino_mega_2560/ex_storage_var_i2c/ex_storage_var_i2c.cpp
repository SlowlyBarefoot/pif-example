// Do not remove the include below
#include "ex_storage_var_i2c.h"
#include "app_main.h"

#include "pif_log.h"

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

BOOL actLogReceiveData(PifComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

void actLedL(SWITCH sw)
{
	digitalWrite(PIN_LED_L, sw);
}

PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	uint8_t error;
#ifdef USE_I2C_WIRE
	int i;
	uint8_t count;
	uint16_t n;

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

    for (n = 0; n < size; n++) {
    	p_data[n] = Wire.read();
    }
#else
	if (!I2C_ReadAddr(addr, iaddr, isize, p_data, size)) {
		error = pif_error;
		goto fail;
	}
#endif
    return IR_COMPLETE;

fail:
	pifLog_Printf(LT_ERROR, "I2CR(%Xh) IA=%lXh IS=%u S=%u E=%u", addr, iaddr, isize, size, error);
	return IR_ERROR;
}

PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	uint8_t error;
#ifdef USE_I2C_WIRE
	int i;
	uint16_t n;

	Wire.beginTransmission(addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (n = 0; n < size; n++) {
    	Wire.write(p_data[n]);
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
	pifLog_Printf(LT_ERROR, "I2CW(%Xh) IA=%lXh IS=%u S=%u E=%u", addr, iaddr, isize, size, error);
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
	I2C_Init(I2C_CLOCK_100KHz);
#endif

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
