#include "appMain.h"
#include "exServoMotor.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifPulse.h"


#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	2
#define TASK_COUNT              3


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer100us = NULL;
PIF_stPulseItem *g_pstPwm = NULL;

static BOOL s_bStart = FALSE;


static uint16_t _taskServoMotor(PIF_stTask *pstTask)
{
	static uint16_t duty = 75;						// 75 = 1.5ms / 20ms * 1000
	static BOOL dir = FALSE;

	(void)pstTask;

	if (!s_bStart) {
		pifPulse_StartItem(g_pstPwm, 200);		// 200 * 100us / 1000 = 20ms
		pifPulse_SetPwmDuty(g_pstPwm, duty);
		s_bStart = TRUE;
	}

	if (g_pstPwm->_enStep == PS_enStop) return 0;

    if (dir) {
        duty -= 5;
        if (duty <= 25) {							// 25 = 0.5ms / 20ms * 1000
        	dir = FALSE;
        }
    }
    else {
        duty += 5;
        if (duty >= 125) {							// 125 = 2.5ms / 20ms * 1000
        	dir = TRUE;
        }
    }
    pifPulse_SetPwmDuty(g_pstPwm, duty);
    return 0;
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stLed *pstLedL;

	pif_Init(NULL);
	pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer100us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 100);		// 100us
    if (!g_pstTimer100us) return;

    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    g_pstPwm = pifPulse_AddItem(g_pstTimer100us, PT_enPwm);
    if (!g_pstPwm) return;
    pifPulse_AttachAction(g_pstPwm, actPulsePwm);

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms

    if (!pifTask_AddPeriodMs(700, _taskServoMotor, NULL)) return;			// 1000ms
}
