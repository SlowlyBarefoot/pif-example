#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"
#include "pifXmodem.h"


PifPulse *g_pstTimer1ms = NULL;

static PifComm *s_pstSerial = NULL;
static PIF_stXmodem *s_pstXmodem = NULL;


static void _evtXmodemRxReceive(uint8_t ucCode, PIF_stXmodemPacket *pstPacket)
{
	if (ucCode == ASCII_SOH) {
		pifLog_Printf(LT_enInfo, "Code=%u PN=%u DT=%2xh", ucCode, (unsigned int)pstPacket->aucPacketNo[0], pstPacket->pucData[0]);
	}
	else {
		pifLog_Printf(LT_enInfo, "Code=%u", ucCode);
	}
}

static void _evtPushSwitchChange(PifId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	if (usLevel) {
	    pifXmodem_ReadyReceive(s_pstXmodem);
	}
}

void appSetup()
{
	PifComm *pstCommLog;
	PifLed *pstLedL;
	PIF_stSensor *pstPushSwitch;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);									// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;				// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;										// 500ms
    pifLed_BlinkOn(pstLedL, 0);

	pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pifComm_AttachActReceiveData(s_pstSerial, actXmodemReceiveData);
	pifComm_AttachActSendData(s_pstSerial, actXmodemSendData);

    s_pstXmodem = pifXmodem_Create(PIF_ID_AUTO, g_pstTimer1ms, XT_enCRC);
    if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvtRxReceive(s_pstXmodem, _evtXmodemRxReceive);
}
