// Do not remove the include below
#include <Wire.h>

#include "exAds1115.h"
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

BOOL actAds1115Write(PifI2c *pstOwner, uint16_t usSize)
{
	uint16_t i;
	BOOL bResult = TRUE;

	Wire.beginTransmission(pstOwner->addr);
    for (i = 0; i < usSize; i++) {
    	Wire.write(pstOwner->p_data[i]);
    }
    uint8_t status_ = Wire.endTransmission();
    if (status_ != 0) {
    	pifLog_Printf(LT_INFO, "I2CW(%d): C=%u, S=%u", pstOwner->p_data[0], usSize);
        bResult = FALSE;
    }
    pifI2c_sigEndWrite(pstOwner, bResult);
    return bResult;
}

BOOL actAds1115Read(PifI2c *pstOwner, uint16_t usSize)
{
	uint16_t i;
	uint8_t count;

    count = Wire.requestFrom(pstOwner->addr, (uint8_t)usSize);
    if (count < usSize) goto fail;

    for (i = 0; i < usSize; i++) {
    	pstOwner->p_data[i] = Wire.read();
    }
    pifI2c_sigEndRead(pstOwner, TRUE);
    return TRUE;

fail:
	pifLog_Printf(LT_INFO, "I2CR(%d): C=%u, S=%u:%u", pstOwner->p_data[0], usSize, count);
    pifI2c_sigEndRead(pstOwner, FALSE);
	return FALSE;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifPulse_sigTick(g_pstTimer1ms);
		return 0;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);

	Wire.begin();

    appSetup(NULL);
    //appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
