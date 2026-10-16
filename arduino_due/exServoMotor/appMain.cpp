#include "appMain.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifTimerManager g_timer_100us;
PifPwmGen g_pwm_gen;

static BOOL s_bStart = FALSE;


static uint32_t _taskServoMotor(PifTask *pstTask)
{
	static uint16_t duty = 75;						// 75 = 1.5ms / 20ms * 1000
	static BOOL dir = FALSE;

	(void)pstTask;

	if (!s_bStart) {
		pifPwmGen_SetDuty(&g_pwm_gen, duty);
		pifPwmGen_Start(&g_pwm_gen, 200);				// 200 * 100us = 20ms
		s_bStart = TRUE;
	}

	if (!g_pwm_gen._running) return 0;

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
    pifPwmGen_SetDuty(&g_pwm_gen, duty);
    return 0;
}

BOOL appSetup()
{
    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 700000, _taskServoMotor, NULL, TRUE)) return FALSE;	// 1000ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;												// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
