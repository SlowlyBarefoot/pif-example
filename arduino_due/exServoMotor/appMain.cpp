#include "appMain.h"
#include "exServoMotor.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_timer.h"


PifTimerManager *g_pstTimer1ms = NULL;
PifTimerManager *g_pstTimer100us = NULL;
PifTimer *g_pstPwm = NULL;

static BOOL s_bStart = FALSE;


static uint16_t _taskServoMotor(PifTask *pstTask)
{
	static uint16_t duty = 75;						// 75 = 1.5ms / 20ms * 1000
	static BOOL dir = FALSE;

	(void)pstTask;

	if (!s_bStart) {
		pifTimer_Start(g_pstPwm, 200);		        // 200 * 100us / 1000 = 20ms
		pifTimer_SetPwmDuty(g_pstPwm, duty);
		s_bStart = TRUE;
	}

	if (g_pstPwm->_step == TS_STOP) return 0;

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
    pifTimer_SetPwmDuty(g_pstPwm, duty);
    return 0;
}

void appSetup()
{
	PifComm *pstCommLog;
	PifLed *pstLedL;

	pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);						// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;					// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer100us = pifTimerManager_Create(PIF_ID_AUTO, 100, 1);						// 100us
    if (!g_pstTimer100us) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;										// 500ms

    g_pstPwm = pifTimerManager_Add(g_pstTimer100us, TT_PWM);
    if (!g_pstPwm) return;
    g_pstPwm->act_pwm = actPulsePwm;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 700, _taskServoMotor, NULL, TRUE)) return;	// 1000ms

    pifLed_BlinkOn(pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse1ms=%d Pulse100us=%d\n", pifTaskManager_Count(),
			pifTimerManager_Count(g_pstTimer1ms), pifTimerManager_Count(g_pstTimer100us));
}
