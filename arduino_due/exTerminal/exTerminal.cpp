// Do not remove the include below
#include "exTerminal.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13


uint16_t actLogSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial.write((char *)pucBuffer, usSize);
}

#ifdef __PIF_LOG_COMMAND__

BOOL actLogReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
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

#endif

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
	pifLog_Printf(LT_enInfo, "LED State=%u", unState);
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

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
