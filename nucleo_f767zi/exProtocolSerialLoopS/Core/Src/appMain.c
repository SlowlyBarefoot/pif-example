#include "appMain.h"
#include "main.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSensorSwitch.h"


#define COMM_COUNT         		3
#define LED_COUNT         		1
#define PROTOCOL_COUNT          2
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              5


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stComm *g_pstCommLog = NULL;


void appSetup()
{
	PIF_stLed *pstLedL;

	pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;
    if (!pifProtocol_Init(PROTOCOL_COUNT, g_pstTimer1ms)) return;

    g_pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
//    if (!pifTask_AddPeriodMs(1, pifComm_taskEach, g_pstCommLog)) return;	// 1ms
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms

    if (!exSerial1_Setup()) return;
    if (!exSerial2_Setup()) return;
}

