#include "appMain.h"
#include "exFnd4S.h"

#include "pifFnd.h"
#include "pifLog.h"


PifPulse *g_pstTimer1ms = NULL;

static PifFnd *s_pstFnd = NULL;

const uint8_t c_ucUserChar[] = { 0x01, 0x08 };


static uint16_t _taskFndTest(PifTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pstTask;

	if (swFloat) {
		s_pstFnd->sub_numeric_digits = 0;
		int32_t nValue = rand() % 14000 - 2000;
		if (nValue <= -1000) {
			pifFnd_SetString(s_pstFnd, (char *)"AAAA");
		}
		else if (nValue < 10000) {
			pifFnd_SetInterger(s_pstFnd, nValue);
		}
		else {
			pifFnd_SetString(s_pstFnd, (char *)"BBBB");
		}

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		s_pstFnd->sub_numeric_digits = 1;
		double dValue = (rand() % 11000 - 1000) / 10.0;
		pifFnd_SetFloat(s_pstFnd, dValue);

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%1f", swBlink, swFloat, dValue);
	}
	swFloat ^= 1;
	nBlink = (nBlink + 1) % 20;
	if (!nBlink) {
		if (swBlink) {
		    pifFnd_BlinkOff(s_pstFnd);
		}
		else {
		    pifFnd_BlinkOn(s_pstFnd, 200);
		}
		swBlink ^= 1;
	}
	return 0;
}

void appSetup()
{
	PifComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;			// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    pifFnd_SetUserChar(c_ucUserChar, 2);
    s_pstFnd = pifFnd_Create(PIF_ID_AUTO, g_pstTimer1ms, 4, actFndDisplay);
    if (!s_pstFnd) return;
    if (!pifFnd_AttachTask(s_pstFnd, TM_RATIO, 5, TRUE)) return;					// 5%

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskFndTest, NULL, TRUE)) return;	// 1000ms

    pifFnd_Start(s_pstFnd);
}
