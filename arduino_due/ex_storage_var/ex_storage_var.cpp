// Do not remove the include below
#include <stdlib.h>

#include "ex_storage_var.h"
#include "app_main.h"


#define PIN_LED_L				13


static DueFlashStorage dueFlashStorage;


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

BOOL actStorageRead(uint8_t* dst, uint32_t src, size_t size, void* p_issuer)
{
	(void)p_issuer;

	memcpy(dst, dueFlashStorage.readAddress(src), size);
	return TRUE;
}

BOOL actStorageWrite(uint32_t dst, uint8_t* src, size_t size, void* p_issuer)
{
	(void)p_issuer;

	return dueFlashStorage.write(dst, src, size);
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

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
