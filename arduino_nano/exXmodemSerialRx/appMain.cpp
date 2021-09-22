#include "appMain.h"
#include "exXmodemSerialRx.h"

#include "pifLed.h"
#include "pifSensorSwitch.h"
#include "pifXmodem.h"


#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define SWITCH_COUNT            1
#define TASK_COUNT              3
#define XMODEM_COUNT            1


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stComm *s_pstSerial = NULL;
static PIF_stXmodem *s_pstXmodem = NULL;


static void _evtPushSwitchChange(PIF_usId usPifId, uint16_t usLevel, void *pvIssuer)
{
	(void)usPifId;
	(void)pvIssuer;

	if (usLevel) {
	    pifXmodem_ReadyReceive(s_pstXmodem);
	}
}

void appSetup()
{
	PIF_stLed *pstLedL;
	PIF_stSensor *pstPushSwitch;

    pif_Init(NULL);

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);						// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;			// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;
    if (!pifXmodem_Init(XMODEM_COUNT, g_pstTimer1ms)) return;

    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;											// 500ms
    pifLed_BlinkOn(pstLedL, 0);

	pstPushSwitch = pifSensorSwitch_Add(PIF_ID_AUTO, 0);
	if (!pstPushSwitch) return;
    if (!pifTask_Add(TM_enPeriodMs, 10, pifSensorSwitch_Task, pstPushSwitch, TRUE)) return;	// 10ms
	pifSensor_AttachAction(pstPushSwitch, actPushSwitchAcquire);
	pifSensor_AttachEvtChange(pstPushSwitch, _evtPushSwitchChange, NULL);

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, s_pstSerial, TRUE)) return;			// 1ms
	pifComm_AttachActReceiveData(s_pstSerial, actXmodemReceiveData);
	pifComm_AttachActSendData(s_pstSerial, actXmodemSendData);

    s_pstXmodem = pifXmodem_Add(PIF_ID_AUTO, XT_enCRC);
    if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
}
