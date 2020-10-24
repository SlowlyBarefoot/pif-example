// Do not remove the include below
#include <MsTimer2.h>

#include "exSolenoid2P.h"

#include "pifLog.h"
#include "pifSolenoid.h"
#include "pifTask.h"


#define PIN_NANO_LED			13

#define PIN_L298N_ENB_PWM		A0
#define PIN_L298N_IN3			6
#define PIN_L298N_IN4			7

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              10
#define SOLENOID_COUNT          1


typedef struct {
	PIF_enSolenoidDir enDir;
    PIF_stSolenoid *pstSolenoid;
    PIF_stPulseItem *pstTimerItem;
} ST_SolenoidTest;


PIF_stPulse *g_pstTimer = NULL;

static ST_SolenoidTest s_stSolenoidTest = { SD_enLeft, NULL, NULL };


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _SolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir)
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

static void _SolenoidEvent(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;
	static BOOL sw = LOW;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 0, pstParam->enDir);

	pifPulse_StartItem(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_enInfo, "_SolenoidEvent(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_enRight ? SD_enLeft : SD_enRight;

	digitalWrite(PIN_NANO_LED, sw);
	sw ^= 1;
}

void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(g_pstTimer);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_NANO_LED, OUTPUT);

	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN3, OUTPUT);
	pinMode(PIN_L298N_IN4, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer = pifPulse_Add(PULSE_ITEM_COUNT);
    if (!g_pstTimer) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_Add(100, pifPulse_LoopAll, NULL)) return;

    if (!pifSolenoid_Init(g_pstTimer, SOLENOID_COUNT)) return;
    s_stSolenoidTest.pstSolenoid = pifSolenoid_Add(0, ST_en2Point, 30, _SolenoidOrder);	// 30 * 1ms = 30ms
    if (!s_stSolenoidTest.pstSolenoid) return;

    s_stSolenoidTest.pstTimerItem = pifPulse_AddItem(g_pstTimer, PT_enOnce);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifPulse_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _SolenoidEvent, &s_stSolenoidTest);
    pifPulse_StartItem(s_stSolenoidTest.pstTimerItem, 1000);		// 1000 * 1ms = 1sec
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}