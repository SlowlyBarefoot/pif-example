// Do not remove the include below
#include <avr/eeprom.h>
#include <MsTimer2.h>

#include "ex_storage_var.h"
#include "app_main.h"


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

BOOL actStorageRead(uint8_t* dst, uint32_t src, size_t size, void* p_issuer)
{
	(void)p_issuer;

	eeprom_read_block(dst, (void *)src, size);
	return TRUE;
}

BOOL actStorageWrite(uint32_t dst, uint8_t* src, size_t size, void* p_issuer)
{
	(void)p_issuer;

	eeprom_write_block(src, (void *)dst, size);
	return TRUE;
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

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
