// Do not remove the include below
#include "exCollectSignal1.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31


static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
} s_stSequenceTest[SEQUENCE_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW }
};


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

void actLedRGBState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(s_stSequenceTest[0].ucPinLed, unState & 1);
	digitalWrite(s_stSequenceTest[1].ucPinLed, (unState >> 1) & 1);
}

uint16_t actPushSwitchAcquire(PIF_usId usPifId)
{
	return !digitalRead(s_stSequenceTest[usPifId - PIF_ID_SWITCH].ucPinSwitch);
}

uint16_t taskTerminal(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(g_pstComm, &txData)) {
    	SerialUSB.print((char)txData);
    }

	rxData = SerialUSB.read();
	if (rxData >= 0) {
		pifComm_ReceiveData(g_pstComm, rxData);
	}
	return 0;
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
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial.begin(115200); //Doesn't matter speed
	SerialUSB.begin(115200); //Doesn't matter speed

	appSetup(micros);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
