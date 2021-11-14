#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"
#include "pif_xmodem.h"


PifTimerManager *g_pstTimer1ms = NULL;

static PifComm *s_pstSerial = NULL;
static PifXmodem *s_pstXmodem = NULL;


static void _evtXmodemRxReceive(uint8_t ucCode, PifXmodemPacket *pstPacket)
{
	if (ucCode == ASCII_SOH) {
		pifLog_Printf(LT_INFO, "Code=%u PN=%u DT=%2xh", ucCode, (unsigned int)pstPacket->packet_no[0], pstPacket->p_data[0]);
	}
	else {
		pifLog_Printf(LT_INFO, "Code=%u", ucCode);
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
	PifSensor *pstPushSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 3);					// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;									// 500ms

	pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_pstSerial->act_receive_data = actXmodemReceiveData;
	s_pstSerial->act_send_data = actXmodemSendData;

    s_pstXmodem = pifXmodem_Create(PIF_ID_AUTO, g_pstTimer1ms, XT_CRC);
    if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvtRxReceive(s_pstXmodem, _evtXmodemRxReceive);

    pifLed_BlinkOn(pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
