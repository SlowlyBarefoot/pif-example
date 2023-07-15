#include "appMain.h"
#include "exSolenoid2P_B.h"

#include "core/pif_log.h"


typedef struct {
	PifSolenoidDir enDir;
    PifSolenoid *pstSolenoid;
    PifTimer *pstTimerItem;
} ST_SolenoidTest;


PifTimerManager g_timer_1ms;

static PifSolenoid s_solenoid;
static ST_SolenoidTest s_stSolenoidTest = { SD_LEFT, &s_solenoid, NULL };


static void _evtSolenoidFinish(void *pvParam)
{
	ST_SolenoidTest *pstParam = (ST_SolenoidTest *)pvParam;

    pifSolenoid_ActionOnDir(pstParam->pstSolenoid, 2500, pstParam->enDir);	// 2500 * 1ms = 2.5sec

	pifTimer_Start(pstParam->pstTimerItem, 1000);	                        // 1000 * 1ms = 1sec

	pifLog_Printf(LT_INFO, "_evtSolenoidFinish(%d)", pstParam->enDir);

	pstParam->enDir = pstParam->enDir == SD_RIGHT ? SD_LEFT : SD_RIGHT;
}

void appSetup()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifSolenoid_Init(&s_solenoid, PIF_ID_AUTO, &g_timer_1ms,
    		ST_2POINT, 30, actSolenoidOrder)) return;								// 30ms
    if (!pifSolenoid_SetBuffer(s_stSolenoidTest.pstSolenoid, 4)) return;

    s_stSolenoidTest.pstTimerItem = pifTimerManager_Add(&g_timer_1ms, TT_ONCE);
    if (!s_stSolenoidTest.pstTimerItem) return;
    pifTimer_AttachEvtFinish(s_stSolenoidTest.pstTimerItem, _evtSolenoidFinish, &s_stSolenoidTest);
    pifTimer_Start(s_stSolenoidTest.pstTimerItem, 1000);					    	// 1000ms

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
