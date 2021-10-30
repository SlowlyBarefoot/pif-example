#include "appMain.h"
#include "exPmlcdI2c.h"

#include "pif_pmlcd_i2c.h"
#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifPulse *g_pstTimer1ms = NULL;

static PifPmlcdI2c *s_pstPmlcdI2c = NULL;
static PifLed *s_pstLedL = NULL;


uint16_t _taskPmlcdI2c(PifTask *pstTask)
{
	static int nStep = 0;
	static int nNumber = 0;

	(void)pstTask;

	pifLog_Printf(LT_INFO, "Task:%u(%u)", __LINE__, nStep);
	switch (nStep) {
	case 0:
		pifPmlcdI2c_LeftToRight(s_pstPmlcdI2c);
		pifPmlcdI2c_SetCursor(s_pstPmlcdI2c, 0, 0);
		pifPmlcdI2c_Print(s_pstPmlcdI2c, "Hello World.");
		pifPmlcdI2c_SetCursor(s_pstPmlcdI2c, 0, 1);
		pifPmlcdI2c_Printf(s_pstPmlcdI2c, "Go Home : %d", nNumber);
		break;

	case 1:
		pifPmlcdI2c_ScrollDisplayLeft(s_pstPmlcdI2c);
		break;

	case 2:
		pifPmlcdI2c_ScrollDisplayRight(s_pstPmlcdI2c);
		break;

	case 3:
		pifPmlcdI2c_Clear(s_pstPmlcdI2c);
		break;

	case 4:
		pifPmlcdI2c_RightToLeft(s_pstPmlcdI2c);
		pifPmlcdI2c_SetCursor(s_pstPmlcdI2c, 15, 0);
		pifPmlcdI2c_Print(s_pstPmlcdI2c, "Hello World.");
		pifPmlcdI2c_SetCursor(s_pstPmlcdI2c, 15, 1);
		pifPmlcdI2c_Printf(s_pstPmlcdI2c, "Go Home : %d", nNumber);
		break;

	case 5:
		pifPmlcdI2c_Clear(s_pstPmlcdI2c);
		break;
	}
	nStep++;
	if (nStep > 5) nStep = 0;
	nNumber++;
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;

    pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstPmlcdI2c = pifPmlcdI2c_Create(PIF_ID_AUTO, 0x27);
    if (!s_pstPmlcdI2c) return;
    s_pstPmlcdI2c->_i2c.act_write = actPmlcdI2cWrite;
#if 0
    pifI2c_ScanAddress(&s_pstPmlcdI2c->_i2c);
#else
    pifPmlcdI2c_Begin(s_pstPmlcdI2c, 2, PIF_PMLCD_DS_5x8);
    pifPmlcdI2c_Backlight(s_pstPmlcdI2c);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 3000, _taskPmlcdI2c, NULL, TRUE)) return;	// 3000ms
#endif
}
