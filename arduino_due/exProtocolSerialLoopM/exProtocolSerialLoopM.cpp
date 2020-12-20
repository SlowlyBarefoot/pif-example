// Do not remove the include below
#include "exProtocolSerialLoopM.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"

#include "exSerial1.h"
#include "exSerial2.h"


#define PIN_LED_L				13

#define COMM_COUNT         		2
#define PROTOCOL_COUNT          2
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              6


PIF_stPulse *g_pstTimer = NULL;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _taskLedToggle(PIF_stTask *pstTask)
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

		pifPulse_sigTick(g_pstTimer);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unDeviceCode unDeviceCode = 1;

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!g_pstTimer) return;

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifProtocol_Init(g_pstTimer, PROTOCOL_COUNT)) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddRatio(3, pifComm_taskAll, NULL)) return;		// 3%

    if (!pifTask_AddPeriod(500, _taskLedToggle, NULL)) return;		// 500ms

    unDeviceCode = exSerial1_Setup(unDeviceCode);
    if (!unDeviceCode) return;

    unDeviceCode = exSerial2_Setup(unDeviceCode);
    if (!unDeviceCode) return;
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
