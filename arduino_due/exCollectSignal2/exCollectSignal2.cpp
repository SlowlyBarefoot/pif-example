// Do not remove the include below
#include "exCollectSignal2.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_BLUE			27
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31
#define PIN_PUSH_SWITCH_3		33


static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

void actGpioRGBState(PifId usPifId, uint8_t unState)
{
	(void)usPifId;

	digitalWrite(s_stSequenceTest[0].ucPinLed, unState & 1);
	digitalWrite(s_stSequenceTest[1].ucPinLed, (unState >> 1) & 1);
}

void actLedCollectState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_BLUE, unState & 1);
}

uint16_t actPushSwitchAcquire(PifId usPifId)
{
	return !digitalRead(s_stSequenceTest[usPifId - PIF_ID_SWITCH].ucPinSwitch);
}

uint16_t actPushSwitchCollectAcquire(PifId usPifId)
{
	(void)usPifId;

	return !digitalRead(PIN_PUSH_SWITCH_3);
}

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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_3, INPUT_PULLUP);

	Serial.begin(115200);

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();
}
