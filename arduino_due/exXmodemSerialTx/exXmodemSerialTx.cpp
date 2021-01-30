// Do not remove the include below
#include "exXmodemSerialTx.h"
#include "appMain.h"


#define PIN_LED_L				13

//#define USE_SERIAL_USB		// Linux or Windows
#define USE_SERIAL_3			// Other Anduino


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void taskXmodemTest(PIF_stTask *pstTask)
{
	uint8_t txData, rxData;

	(void)pstTask;

    while (pifComm_SendData(g_pstSerial, &txData)) {
#ifdef USE_SERIAL_USB
    	SerialUSB.print((char)txData);
#endif
#ifdef USE_SERIAL_3
    	Serial3.print((char)txData);
#endif
    }

    while (pifComm_GetRemainSizeOfRxBuffer(g_pstSerial)) {
#ifdef USE_SERIAL_USB
		if (SerialUSB.available()) {
			rxData = SerialUSB.read();
			pifComm_ReceiveData(g_pstSerial, rxData);
		}
		else break;
#endif
#ifdef USE_SERIAL_3
		if (Serial3.available()) {
			rxData = Serial3.read();
			pifComm_ReceiveData(g_pstSerial, rxData);
		}
		else break;
#endif
    }
}

void taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
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

	Serial.begin(115200); //Doesn't matter speed
#ifdef USE_SERIAL_USB
	SerialUSB.begin(115200);
#endif
#ifdef USE_SERIAL_3
	Serial3.begin(9600);
#endif

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
