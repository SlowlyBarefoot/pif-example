#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_sensor_switch.h"
#include "pif_xmodem.h"


PifTimerManager g_timer_1ms;

static PifComm s_serial;
static PifXmodem s_xmodem;


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
	    pifXmodem_ReadyReceive(&s_xmodem);
	}
}

void appSetup()
{
	static PifComm s_comm_log;
	static PifLed s_led_l;
	PifSensorSwitch stPushSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

	if (!pifSensorSwitch_Init(&stPushSwitch, PIF_ID_AUTO, 0)) return;
    if (!pifSensorSwitch_AttachTask(&stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
	pifSensor_AttachAction(&stPushSwitch.parent, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(&stPushSwitch.parent, _evtPushSwitchChange, NULL);

	if (!pifComm_Init(&s_serial, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
    s_serial.act_receive_data = actXmodemReceiveData;
    s_serial.act_send_data = actXmodemSendData;

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return;
    pifXmodem_AttachComm(&s_xmodem, &s_serial);
    pifXmodem_AttachEvtRxReceive(&s_xmodem, _evtXmodemRxReceive);

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
