// Do not remove the include below
#include "ex_gy86.h"
#include "app_main.h"

#include "core/pif_log.h"

#ifdef USE_I2C_WIRE
	#include <Wire.h>
#else
	#include "../i2c.h"
#endif


#define PIN_LED_L				13


#ifdef __PIF_DEBUG__

static void actTaskYield()
{
	static BOOL state = FALSE;
	digitalWrite(52, state);
	state ^= 1;
}

#endif

uint16_t actLogSendData(PifComm* p_comm, uint8_t* p_buffer, uint16_t size)
{
	(void)p_comm;

    return Serial.write((char *)p_buffer, size);
}

void actLedLState(PifId id, uint32_t state)
{
	(void)id;

	digitalWrite(PIN_LED_L, state & 1);
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
    for (i = 0; i < (int)size; i++) {
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
	pifLog_Printf(LT_ERROR, "I2CW(%Xh): C=%Xh S=%u E=%u", addr, p_data[0], size, error);
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

    for (i = 0; i < (int)size; i++) {
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
	pifLog_Printf(LT_ERROR, "I2CR(%Xh): C=%Xh S=%u E=%u", addr, p_data[0], size, error);
	return IR_ERROR;
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
#ifdef __PIF_DEBUG__
	pinMode(52, OUTPUT);
#endif

	Serial.begin(115200);

#ifdef USE_I2C_WIRE
	Wire.begin();
	Wire.setClock(400000);
#else
	I2C_Init(I2C_CLOCK_400KHz);
#endif

#ifdef __PIF_DEBUG__
	pif_act_task_yield = actTaskYield;
#endif

    appSetup(NULL);
    //appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
