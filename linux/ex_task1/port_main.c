#include "host.h"
#include "app_port.h"


static uint32_t _taskLedToggle(PifTask *pstTask)
{
	static int nCount = 0;
	static BOOL sw = FALSE;

	(void)pstTask;

    if (!nCount) {
    	hostLed(sw);
		sw ^= 1;

		nCount = 499;
    }
    else nCount--;
    return 0;
}

// Body of the SysTick handler of a board, called from the tick thread of host.c.
void portTick1ms()
{
	pif_sigTimer1ms();
}

BOOL portSetup()
{
	if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;
	pifTaskManager_SetIdle(hostEvtIdle, 0);

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskLedToggle, NULL, TRUE)) return FALSE;	// 1ms
	return TRUE;
}
