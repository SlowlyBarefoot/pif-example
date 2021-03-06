#include "appMain.h"
#include "exProtocolSerialLoopS.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifProtocol.h"

#include "exSerial1.h"
#include "exSerial2.h"


#define COMM_COUNT         		2
#define LED_COUNT         		1
#define PROTOCOL_COUNT          2
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;


void appSetup()
{
	PIF_stLed *pstLedL = NULL;

	pif_Init(NULL);

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);	// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifProtocol_Init(g_pstTimer1ms, PROTOCOL_COUNT)) return;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddRatio(3, pifComm_taskAll, NULL)) return;			// 3%

    if (!exSerial1_Setup()) return;
    if (!exSerial2_Setup()) return;
}
