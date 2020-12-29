// Do not remove the include below
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

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },

	{ NULL, NULL, NULL }
};


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

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

		nCount = 99;
    }
    else nCount--;

    while (pifComm_SendData(s_pstComm, &txData)) {
    	SerialUSB.print((char)txData);
    }

    while (1) {
		rxData = SerialUSB.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(s_pstComm, rxData);
		}
		else break;
    }
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
	SerialUSB.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
	s_pstComm = pifComm_Add(0);
	if (!s_pstComm) return;

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(s_pstComm);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddPeriodMs(10, pifComm_taskAll, NULL)) return;	// 10ms
    if (!pifTask_AddPeriodMs(10, _taskTerminal, NULL)) return;		// 10ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
