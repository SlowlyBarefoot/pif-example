// Do not remove the include below
#include <MsTimer2.h>

#include "exTerminal.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifTask.h"
#include "pifTerminal.h"


#define PIN_LED_L				13

#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


static PIF_stComm *s_pstComm = NULL;

static PIF_stTerminal *s_pstTerminal = NULL;

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },

	{ NULL, NULL, NULL }
};


static void _taskTerminal(PIF_stTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = LOW;
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (!nCount) {
		digitalWrite(PIN_LED_L, sw);
		sw ^= 1;

		nCount = 59;
    }
    else nCount--;

    while (pifComm_SendData(s_pstComm, &txData)) {
    	Serial.print((char)txData);
    }

    while (1) {
		rxData = Serial.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(s_pstComm, rxData);
		}
		else break;
    }
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

    pif_Init();

    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
	s_pstComm = pifComm_Add(0);
	if (!s_pstComm) return;

	s_pstTerminal = pifTerminal_Init(c_psCmdTable, "\nDebug");
    if (!s_pstTerminal) return;
	pifTerminal_AttachComm(s_pstComm);

    pifLog_AttachTerminal(s_pstTerminal);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriod(10, pifComm_taskAll, NULL)) return;		// 10ms
    if (!pifTask_AddPeriod(10, _taskTerminal, NULL)) return;		// 10ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
