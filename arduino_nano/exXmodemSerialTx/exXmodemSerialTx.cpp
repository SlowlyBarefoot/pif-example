// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exXmodemSerialTx.h"
#include "appMain.h"


#define PIN_LED_L				13


static SoftwareSerial SwSerial(7, 8);


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void taskXmodemTest(PIF_stTask *pstTask)
{
	uint8_t txData, rxData;

	(void)pstTask;

    while (pifComm_SendData(g_pstSerial, &txData)) {
    	SwSerial.write((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(g_pstSerial)) {
    	if (SwSerial.available()) {
			rxData = SwSerial.read();
			pifComm_ReceiveData(g_pstSerial, rxData);
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

	pifPulse_sigTick(g_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed
	SwSerial.begin(115200);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
