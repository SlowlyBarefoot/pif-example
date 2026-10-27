#include "app_port.h"


#define MAX_CHANNEL		8

#define PULSE_MIN   (750)       // minimum PWM pulse width which is considered valid
#define PULSE_MAX   (2250)      // maximum PWM pulse width which is considered valid


PifRcPpm g_rc_ppm;
PifTimerManager g_timer_1ms;

static uint16_t s_value[MAX_CHANNEL];
static BOOL s_failsafe = FALSE;


static void _evtRcPpmReceive(PifRc* p_owner, uint16_t* p_channel, PifIssuerP p_issuer)
{
	int i;
	PifTask* p_task = (PifTask*)p_issuer;

	for (i = 0; i < p_owner->_channel_count; i++) {
		s_value[i] = p_channel[i];
	}
	pifTask_SetTrigger(p_task, 0);
}

static uint32_t _taskPrint(PifTask* p_task)
{
	(void)p_task;

	pifLog_Printf(LT_INFO, "1:%4u 2:%4u 3:%4u 4:%4u 5:%4u 6:%4u 7:%4u 8:%4u",
			s_value[0], s_value[1], s_value[2], s_value[3],	s_value[4], s_value[5], s_value[6], s_value[7]);
    return 0;
}

static uint32_t _taskFailsafe(PifTask* p_task)
{
	BOOL failsafe = pifRc_CheckFailSafe(&g_rc_ppm.parent);

	(void)p_task;

	if (failsafe != s_failsafe) {
		s_failsafe = failsafe;
		pifLog_Printf(LT_INFO, "Failsafe:%d Loss:%d%%", failsafe, pifRc_GetFrameLoss(&g_rc_ppm.parent));
	}
    return 0;
}

BOOL appSetup()
{
	PifTask* p_task;

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_EXTERNAL, 0, _taskPrint, NULL, FALSE);
    if (!p_task) return FALSE;

    if (!pifRcPpm_Init(&g_rc_ppm, PIF_ID_AUTO, MAX_CHANNEL, 2700)) return FALSE;
    pifRcPpm_SetValidRange(&g_rc_ppm, PULSE_MIN, PULSE_MAX);
    pifRc_AttachEvtReceive(&g_rc_ppm.parent, _evtRcPpmReceive, p_task);

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 100000, _taskFailsafe, NULL, TRUE)) return FALSE;	// 100ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_rc_ppm             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
