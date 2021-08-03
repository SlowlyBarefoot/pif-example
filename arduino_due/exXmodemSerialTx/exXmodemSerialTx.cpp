// Do not remove the include below
#include "exXmodemSerialTx.h"
#include "appMain.h"


#define PIN_LED_L				13


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actXmodemSendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial3.write((char *)pucBuffer, usSize);
}

BOOL actXmodemReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial3.read();
	if (rxData >= 0) {
		if (rxData == 'C') Serial.write('C');
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
	Serial3.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
