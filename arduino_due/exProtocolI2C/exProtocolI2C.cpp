// Do not remove the include below
#include <Wire.h>

#include "exProtocolI2C.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

SWITCH actPushSwitchAcquire(PIF_usId usPifId)
{
	return digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;
	(void)ucIndex;

	digitalWrite(PIN_LED_L, swState);
}

void taskI2C(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;
	static int stepRead = 0;

	(void)pstTask;

	if (pifComm_GetFillSizeOfTxBuffer(g_pstI2C)) {
		Wire.beginTransmission(1);
		for (int i = 0; i < PIF_COMM_RX_BUFFER_SIZE; i++) {
			if (pifComm_SendData(g_pstI2C, &txData)) {
				Wire.write((char)txData);
				if (txData == ASCII_ETX) {
					stepRead = 1;
				}
			}
			else break;
		}
		Wire.endTransmission();
	}

    switch (stepRead) {
    case 1:
    	if (pifComm_GetRemainSizeOfRxBuffer(g_pstI2C)) {
			Wire.requestFrom(1, 1);
			stepRead = 2;
    	}
        break;

    case 2:
    	if (Wire.available() > 0) {
			rxData = Wire.read();
			if (rxData > 0) {
				pifComm_ReceiveData(g_pstI2C, rxData);
				switch (rxData) {
				case ASCII_ETX:
				case ASCII_ACK:
				case ASCII_NAK:
					stepRead = 0;
					break;

				default:
					stepRead = 1;
					break;
				}
			}
    	}
    	break;
    }
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
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(115200);

	Wire.begin();

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
