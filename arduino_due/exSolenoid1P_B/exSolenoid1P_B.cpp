// Do not remove the include below
#include "exSolenoid1P_B.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13

#define PIN_RELAY_1CH			48


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actSolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir)
{
	(void)enDir;

	digitalWrite(PIN_RELAY_1CH, swOrder);

	pifLog_Printf(LT_enInfo, "_SolenoidOrder(%d)", swOrder);
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

	pinMode(PIN_RELAY_1CH, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
