#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pif_led.h"
#include "pif_sensor_switch.h"
#include "pif_xmodem.h"


PifTimerManager g_timer_1ms;

static PifComm s_serial;
static PifXmodem s_xmodem;


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
	static PifLed s_led_l;
	PifSensor *pstPushSwitch;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;			// 1000us

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

	pstPushSwitch = pifSensorSwitch_Create(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifSensorSwitch_AttachTask(pstPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

	if (!pifComm_Init(&s_serial, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
    s_serial.act_receive_data = actXmodemReceiveData;
    s_serial.act_send_data = actXmodemSendData;

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return;
    pifXmodem_AttachComm(&s_xmodem, &s_serial);

    pifLed_BlinkOn(&s_led_l, 0);
}
