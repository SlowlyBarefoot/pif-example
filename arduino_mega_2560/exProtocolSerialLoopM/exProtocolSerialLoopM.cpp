// Do not remove the include below
#include <MsTimer2.h>

#include "exProtocolSerialLoopM.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static uint8_t s_ucPinSwitch[SWITCH_COUNT] = { PIN_PUSH_SWITCH_1, PIN_PUSH_SWITCH_2 };


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	return !digitalRead(s_ucPinSwitch[usPifId - PIF_ID_SWITCH]);
}

uint16_t actSerial1SendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial1.write((char *)pucBuffer, usSize);
}

BOOL actSerial1ReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial1.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
}

uint16_t actSerial2SendData(PIF_stComm *pstComm, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)pstComm;

    return Serial2.write((char *)pucBuffer, usSize);
}

BOOL actSerial2ReceiveData(PIF_stComm *pstComm, uint8_t *pucData)
{
	int rxData;

	(void)pstComm;

	rxData = Serial2.read();
	if (rxData >= 0) {
		*pucData = rxData;
		return TRUE;
	}
	return FALSE;
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
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

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
