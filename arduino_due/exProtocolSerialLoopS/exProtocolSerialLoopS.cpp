// Do not remove the include below
#include "exProtocolSerialLoopS.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint8_t ucIndex, SWITCH swState)
{
	(void)usPifId;
	(void)ucIndex;

	digitalWrite(PIN_LED_L, swState);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	return !digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

void taskSerial1(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(g_pstSerial1, &txData)) {
    	Serial1.print((char)txData);
    }

    if (pifComm_GetRemainSizeOfRxBuffer(g_pstSerial1)) {
		rxData = Serial1.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(g_pstSerial1, rxData);
		}
    }
}

void taskSerial2(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(g_pstSerial2, &txData)) {
    	Serial2.print((char)txData);
    }

    if (pifComm_GetRemainSizeOfRxBuffer(g_pstSerial2)) {
		rxData = Serial2.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(g_pstSerial2, rxData);
		}
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
	Serial1.begin(115200);
	Serial2.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
