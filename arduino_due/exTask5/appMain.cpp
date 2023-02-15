#include "appMain.h"
#include "exTask5.h"

#include "core/pif_log.h"


PifComm g_comm_log;

static PifTask* s_task;
static int s_state = 0;


static uint16_t _taskLedToggle(PifTask *pstTask)
{
	static int count = 0;
	static BOOL sw = LOW;

	(void)pstTask;

	if (count) {
		actLedL(sw);
		sw ^= 1;

		count--;
	}
	else {
		switch (s_state) {
		case 0:
			pifTask_ChangeMode(s_task, TM_PERIOD_MS, 1);
			s_state++;
			break;

		case 1:
			pifTask_ChangeMode(s_task, TM_RATIO, 50);
			s_state = 0;
			break;
		}
		count = 100;
#ifdef __PIF_DEBUG__
		pifTaskManager_PrintRatioTable();
#endif
	}

    return 0;
}

void appSetup()
{
	pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE, NULL)) return;	// 1ms
	g_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&g_comm_log)) return;

	s_task = pifTaskManager_Add(TM_RATIO, 50, _taskLedToggle, NULL, TRUE);		// 50%
	if (!s_task) return;

#ifdef __PIF_DEBUG__
	pifTaskManager_PrintRatioTable();
#endif

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
