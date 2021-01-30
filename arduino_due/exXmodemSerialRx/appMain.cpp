#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pifLog.h"
#include "pifXmodem.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              4
#define XMODEM_COUNT            1


PIF_stComm *g_pstSerial = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stXmodem *s_pstXmodem = NULL;


static void _evtXmodemRxReceive(uint8_t ucCode, PIF_stXmodemPacket *pstPacket)
{
	if (ucCode == ASCII_SOH) {
		pifLog_Printf(LT_enInfo, "Code=%u PN=%u", ucCode, (unsigned int)pstPacket->aucPacketNo[0]);
	}
	else {
		pifLog_Printf(LT_enInfo, "Code=%u", ucCode);
	}
}

void appSetup()
{
    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);	// 1000us
    if (!g_pstTimer1ms) return;

    g_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstSerial) return;

    if (!pifXmodem_Init(g_pstTimer1ms, XMODEM_COUNT)) return;
    s_pstXmodem = pifXmodem_Add(PIF_ID_AUTO, XT_enCRC);
    if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, g_pstSerial);
    pifXmodem_AttachEvent(s_pstXmodem, NULL, _evtXmodemRxReceive);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodUs(500, pifComm_taskAll, NULL)) return;		// 500us

    if (!pifTask_AddPeriodUs(500, taskXmodemTest, NULL)) return;		// 500us
    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;			// 500ms

    pifXmodem_ReadyReceive(s_pstXmodem);
}
