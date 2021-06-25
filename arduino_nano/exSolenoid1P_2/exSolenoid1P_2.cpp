// Do not remove the include below
#include <MsTimer2.h>

#include "exSolenoid1P_2.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13

#define PIN_RELAY_1CH			5


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

uint16_t taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
	return 0;
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

	pinMode(PIN_RELAY_1CH, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
