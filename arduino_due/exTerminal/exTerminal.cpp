// Do not remove the include below
#include "exTerminal.h"
#include "appMain.h"

#include "core/pif_log.h"


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

void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	pifLog_Printf(LT_INFO, "LED State=%u", unState);
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

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
