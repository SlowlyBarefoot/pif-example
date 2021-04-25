// Do not remove the include below
#include <MsTimer2.h>
#include <Wire.h>

#include "exProtocolI2C.h"
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

static void receiveEvent(int parameter)
{
	uint8_t rxData;

	(void)parameter;

	int size = pifComm_GetRemainSizeOfRxBuffer(g_pstI2C);
    for (int i = 0; i < size; i++) {
    	if (Wire.available() > 0) {
			rxData = Wire.read();
			pifComm_ReceiveData(g_pstI2C, rxData);
    	}
		else break;
    }
}

static void requestEvent()
{
	uint8_t txData = 0;

	pifComm_SendData(g_pstI2C, &txData);
	Wire.write((char)txData);
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

	Wire.begin(1);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

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
