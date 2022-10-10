#include "app_main.h"
#include "ex_i2c_scan.h"

#include "core/pif_log.h"


PifTimerManager g_timer_1ms;

static PifI2cPort s_i2c_port;


void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 1, 16)) return;
    s_i2c_port.act_write = actI2cWrite;

    pifI2cPort_ScanAddress(&s_i2c_port);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
