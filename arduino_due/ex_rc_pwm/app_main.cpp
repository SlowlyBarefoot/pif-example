#include "app_main.h"
#include "ex_rc_pwm.h"

#include "core/pif_log.h"


#define MAX_CHANNEL		6

#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid


PifRcPwm g_rc_pwm;
PifTimerManager g_timer_1ms;

static uint16_t s_value[MAX_CHANNEL];


static void _evtRcPwmReceive(PifRc* p_owner, uint16_t* p_channel, PifIssuerP p_issuer)
{
	int i;
	PifTask* p_task = (PifTask*)p_issuer;

	for (i = 0; i < p_owner->_channel_count; i++) {
		s_value[i] = p_channel[i];
	}
	pifTask_SetTrigger(p_task);
}

static uint16_t _taskPrint(PifTask* p_task)
{
	(void)p_task;

	pifLog_Printf(LT_INFO, "1:%4u 2:%4u 3:%4u 4:%4u 5:%4u 6:%4u",
			s_value[0], s_value[1], s_value[2], s_value[3],	s_value[4], s_value[5]);
    return 0;
}

static uint16_t _taskLedToggle(PifTask* p_task)
{
	static BOOL sw = FALSE;

	(void)p_task;

   	actLedL(sw);
	sw ^= 1;
    return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	PifTask* p_task;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE, NULL)) return;		// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

	p_task = pifTaskManager_Add(TM_EXTERNAL_ORDER, 0, _taskPrint, NULL, FALSE);
    if (!p_task) return;

    if (!pifRcPwm_Init(&g_rc_pwm, PIF_ID_AUTO, MAX_CHANNEL)) return;
    pifRcPwm_SetValidRange(&g_rc_pwm, PULSE_MIN, PULSE_MAX);
    pifRc_AttachEvtReceive(&g_rc_pwm.parent, _evtRcPwmReceive, p_task);

	if (!pifTaskManager_Add(TM_PERIOD_MS, 100, _taskLedToggle, NULL, TRUE)) return;	// 100ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
