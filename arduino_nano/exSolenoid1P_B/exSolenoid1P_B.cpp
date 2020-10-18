// Do not remove the include below
#include <MsTimer2.h>

#include "exSolenoid1P_B.h"

#include "pifLog.h"
#include "pifSolenoid.h"
#include "pifTask.h"


#define PIN_NANO_LED			13

#define PIN_RELAY_1CH			5

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              10
#define SOLENOID_COUNT          1


typedef struct {
    PIF_stSolenoid *pstSolenoid;
    PIF_stPulseItem *pstTimerItem;
} ST_SolenoidTest;


PIF_stPulse *g_pstTimer = NULL;

static ST_SolenoidTest s_stSolenoidTest = {	NULL, NULL };


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _SolenoidOrder(SWITCH swOrder, PIF_enSolenoidDir enDir)
{
	(void)enDir;

	digitalWrite(PIN_RELAY_1CH, swOrder);

	pifLog_Printf(LT_enInfo, "_SolenoidOrder(%d)", swOrder);
}

static void _SolenoidEvent(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;
	static BOOL sw = LOW;

	pifSolenoid_ActionOn(pstParam->pstSolenoid, 2500);	// 2500 * 1ms = 2.5sec

	pifPulse_StartItem(pstParam->pstTimerItem, 1000);	// 1000 * 1ms = 1sec

	pifLog_Printf(LT_enInfo, "_SolenoidEvent()");

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

	pinMode(PIN_RELAY_1CH, OUTPUT);

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
    s_stSolenoidTest.pstSolenoid = pifSolenoid_Add(0, ST_en1Point, 300, _SolenoidOrder);	// 300 * 1ms = 300ms
    if (!s_stSolenoidTest.pstSolenoid) return;
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return;

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
