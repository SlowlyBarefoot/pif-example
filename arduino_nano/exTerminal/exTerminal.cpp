// Do not remove the include below
#include <MsTimer2.h>

#include "exTerminal.h"
#include "appMain.h"


#define PIN_LED_L				13


void taskTerminal(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    while (pifComm_SendData(g_pstComm, &txData)) {
    	Serial.print((char)txData);
    }

    while (1) {
		rxData = Serial.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(g_pstComm, rxData);
		}
		else break;
    }
}

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void sysTickHook()
{
	pif_sigTimer1ms();
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
    pif_Loop();

    pifTask_Loop();
}
