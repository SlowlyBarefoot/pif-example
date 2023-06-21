// Do not remove the include below
#include "ex_gps_i2c_ublox.h"
#include "linker.h"

#include <Wire.h>


#define PIN_LED_L				13


void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actLogSendData(PifComm *pstOwner, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstOwner;

    return Serial.write((char *)pucBuffer, usSize);
}

BOOL actLogReceiveData(PifComm *pstOwner, uint8_t *pucData)
{
	int rxData;

	(void)pstOwner;

	rxData = Serial.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

PifI2cReturn actI2cWrite(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	int i;

	Wire1.beginTransmission(addr);
	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			Wire1.write((iaddr >> (i * 8)) & 0xFF);
		}
	}
    for (i = 0; i < (int)size; i++) {
    	Wire1.write(p_data[i]);
    }
    if (Wire1.endTransmission() != 0) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}
    return IR_COMPLETE;
}

PifI2cReturn actI2cRead(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint16_t size)
{
	int i;
	uint8_t count;

	if (isize > 0) {
		Wire1.beginTransmission(addr);
		for (i = isize - 1; i >= 0; i--) {
			Wire1.write((iaddr >> (i * 8)) & 0xFF);
		}
	    if (Wire1.endTransmission() != 0) {
			pif_error = E_TRANSFER_FAILED;
			return IR_ERROR;
		}
	}

    count = Wire1.requestFrom(addr, (uint8_t)size);
    if (count < size) {
		pif_error = E_TRANSFER_FAILED;
		return IR_ERROR;
	}

    for (i = 0; i < (int)size; i++) {
    	p_data[i] = Wire1.read();
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

	Wire1.begin();
//	Wire1.setClock(400000);

    appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
