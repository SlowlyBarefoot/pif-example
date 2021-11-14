#include "appMain.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifComm *g_pstCommLog = NULL;
PifPmlcdI2c *g_pstPmlcdI2c = NULL;
PifTimerManager *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL = NULL;


uint16_t _taskPmlcdI2c(PifTask *pstTask)
{
	static int nStep = 0;
	static int nNumber = 0;

	(void)pstTask;

	pifLog_Printf(LT_INFO, "Task:%u(%u) %lu", __LINE__, nStep, (*pif_act_timer1us)());
	switch (nStep) {
	case 0:
		pifPmlcdI2c_LeftToRight(g_pstPmlcdI2c);
		pifPmlcdI2c_SetCursor(g_pstPmlcdI2c, 0, 0);
		pifPmlcdI2c_Print(g_pstPmlcdI2c, "Hello World.");
		pifPmlcdI2c_SetCursor(g_pstPmlcdI2c, 0, 1);
		pifPmlcdI2c_Printf(g_pstPmlcdI2c, "Go Home : %d", nNumber);
		break;

	case 1:
		pifPmlcdI2c_ScrollDisplayLeft(g_pstPmlcdI2c);
		break;

	case 2:
		pifPmlcdI2c_ScrollDisplayRight(g_pstPmlcdI2c);
		break;

	case 3:
		pifPmlcdI2c_Clear(g_pstPmlcdI2c);
		break;

	case 4:
		pifPmlcdI2c_RightToLeft(g_pstPmlcdI2c);
		pifPmlcdI2c_SetCursor(g_pstPmlcdI2c, 15, 0);
		pifPmlcdI2c_Print(g_pstPmlcdI2c, "Hello World.");
		pifPmlcdI2c_SetCursor(g_pstPmlcdI2c, 15, 1);
		pifPmlcdI2c_Printf(g_pstPmlcdI2c, "Go Home : %d", nNumber);
		break;

	case 5:
		pifPmlcdI2c_Clear(g_pstPmlcdI2c);
		break;
	}
	nStep++;
	if (nStep > 5) nStep = 0;
	nNumber++;
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 1);					// 1000us
    if (!g_pstTimer1ms) return;

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	g_pstCommLog->act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms

    g_pstPmlcdI2c = pifPmlcdI2c_Create(PIF_ID_AUTO, 0x27);
    if (!g_pstPmlcdI2c) return;
    g_pstPmlcdI2c->_i2c.act_write = actPmlcdI2cWrite;
#if 0
    pifI2c_ScanAddress(&g_pstPmlcdI2c->_i2c);
#else
    pifPmlcdI2c_Begin(g_pstPmlcdI2c, 2, PIF_PMLCD_DS_5x8);
    pifPmlcdI2c_Backlight(g_pstPmlcdI2c);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 3000, _taskPmlcdI2c, NULL, TRUE)) return;	// 3000ms
#endif

    pifLed_BlinkOn(s_pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
}
