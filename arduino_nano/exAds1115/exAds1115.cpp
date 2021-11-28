// Do not remove the include below
#include "exAds1115.h"
#include "appMain.h"
#ifndef I2C_WIRE_LIB
	#include "../i2c.h"
#endif

#include "pif_log.h"

#include <MsTimer2.h>
#ifdef I2C_WIRE_LIB
	#include <Wire.h>
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

BOOL actAds1115Write(PifI2c *pstOwner, uint16_t usSize)
{
#ifdef I2C_WIRE_LIB
	uint16_t i;

	Wire.beginTransmission(pstOwner->addr);
    for (i = 0; i < usSize; i++) {
    	Wire.write(pstOwner->p_data[i]);
    }
    if (Wire.endTransmission() != 0) goto fail;
#else
	if (!I2C_Start(pstOwner->addr, I2C_MODE_WRITE)) goto fail;
	if (!I2C_Write(pstOwner->p_data, usSize)) goto fail;
	I2C_Stop(1);
#endif
    pifI2c_sigEndWrite(pstOwner, TRUE);
    return TRUE;

fail:
	pifI2c_sigEndWrite(pstOwner, FALSE);
	pifLog_Printf(LT_INFO, "I2CW(%d): C=%u, S=%u", pstOwner->p_data[0], usSize);
	return FALSE;
}

BOOL actAds1115Read(PifI2c *pstOwner, uint16_t usSize)
{
#ifdef I2C_WIRE_LIB
	uint16_t i;
	uint8_t count;

    count = Wire.requestFrom(pstOwner->addr, (uint8_t)usSize);
    if (count < usSize) goto fail;

    for (i = 0; i < usSize; i++) {
    	pstOwner->p_data[i] = Wire.read();
    }
#else
	if (!I2C_Start(pstOwner->addr, I2C_MODE_READ)) goto fail;
	if (!I2C_Read(pstOwner->p_data, usSize)) goto fail;
	I2C_Stop(1);
#endif
    pifI2c_sigEndRead(pstOwner, TRUE);
    return TRUE;

fail:
	pifI2c_sigEndRead(pstOwner, FALSE);
	pifLog_Printf(LT_INFO, "I2CR(%d): C=%u, S=%u", pstOwner->p_data[0], usSize);
	return FALSE;
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

#ifdef I2C_WIRE_LIB
	Wire.begin();
#else
	I2C_Init(I2C_CLOCK_400KHz);
#endif

    appSetup(NULL);
    //appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
