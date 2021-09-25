#include "appMain.h"
#include "main.h"

#include "pifLed.h"
#include "pifLog.h"


#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              3

#define SINGLE_SHOT


PIF_stComm *g_pstCommLog = NULL;
PIF_stPmlcdI2c *g_pstPmlcdI2c = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;


uint16_t _taskPmlcdI2c(PIF_stTask *pstTask)
{
	static int nStep = 0;
	static int nNumber = 0;

	(void)pstTask;

	pifLog_Printf(LT_enInfo, "Task:%u(%u) %lu", __LINE__, nStep, (*pif_actTimer1us)());
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

void appSetup(PIF_actTimer1us actTimer1us)
{
    pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Init(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);			// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;		// 100%

    g_pstCommLog = pifComm_Init(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_enPeriodMs, 1, TRUE)) return;		// 1ms
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    s_pstLedL = pifLed_Init(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    g_pstPmlcdI2c = pifPmlcdI2c_Create(PIF_ID_AUTO, 0x27);
    if (!g_pstPmlcdI2c) return;
    pifI2c_AttachAction(&g_pstPmlcdI2c->_stI2c, NULL, actPmlcdI2cWrite);
#if 0
    pifI2c_ScanAddress(&g_pstPmlcdI2c->_stI2c);
#else
    pifPmlcdI2c_Begin(g_pstPmlcdI2c, 2, PIF_PMLCD_DS_5x8);
    pifPmlcdI2c_Backlight(g_pstPmlcdI2c);

    if (!pifTask_Add(TM_enPeriodMs, 3000, _taskPmlcdI2c, NULL, TRUE)) return;	// 3000ms
#endif
}
