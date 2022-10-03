#include "appMain.h"
#include "exServoMotor.h"

#include "core/pif_log.h"
#include "core/pif_timer.h"
#include "display/pif_led.h"


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_100us;
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
	static PifComm s_comm_log;
	static PifLed s_led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;				// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifTimerManager_Init(&g_timer_100us, PIF_ID_AUTO, 100, 1)) return;				// 100us

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;									// 500ms

    g_pstPwm = pifTimerManager_Add(&g_timer_100us, TT_PWM);
    if (!g_pstPwm) return;
    g_pstPwm->act_pwm = actPulsePwm;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 700, _taskServoMotor, NULL, TRUE)) return;	// 1000ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer 1ms=%d Timer 100us=%d\n", pifTaskManager_Count(),
			pifTimerManager_Count(&g_timer_1ms), pifTimerManager_Count(&g_timer_100us));
}
