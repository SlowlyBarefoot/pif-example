// Do not remove the include below
#include <MsTimer2.h>

#include "exSolenoid2P.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13

#define PIN_L298N_ENB_PWM		A0
#define PIN_L298N_IN3			6
#define PIN_L298N_IN4			7


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void actSolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir)
{
	if (swOrder) {
		switch (enDir) {
		case SD_enLeft:
			digitalWrite(PIN_L298N_IN3, HIGH);
			digitalWrite(PIN_L298N_IN4, LOW);
			break;

		case SD_enRight:
			digitalWrite(PIN_L298N_IN3, LOW);
			digitalWrite(PIN_L298N_IN4, HIGH);
			break;

		default:
			break;
		}
		analogWrite(PIN_L298N_ENB_PWM, 255);
	}
	else {
		analogWrite(PIN_L298N_ENB_PWM, 0);
	}
	pifLog_Printf(LT_enInfo, "_SolenoidOrder(%d, %d)", enDir, swOrder);
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

	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN3, OUTPUT);
	pinMode(PIN_L298N_IN4, OUTPUT);

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
