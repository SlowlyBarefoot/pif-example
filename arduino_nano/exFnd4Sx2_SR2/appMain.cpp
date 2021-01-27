#include "appMain.h"
#include "exFnd4Sx2_SR2.h"

#include "pifFnd.h"
#include "pifLog.h"


#define FND_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              4


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stFnd *s_pstFnd = NULL;

const uint8_t c_ucUserChar[] = { 0x01, 0x08 };


static void _taskFndTest(PIF_stTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;

	(void)pstTask;

	if (swFloat) {
		s_pstFnd->ucSubNumericDigits = 0;
		int32_t nValue = rand() % 140000 - 20000;
		if (nValue <= -10000) {
			pifFnd_SetString(s_pstFnd, (char *)"AAAAAAAA");
		}
		else if (nValue < 100000) {
			pifFnd_SetInterger(s_pstFnd, nValue);
		}
		else {
			pifFnd_SetString(s_pstFnd, (char *)"BBBBBBBB");
		}

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		s_pstFnd->ucSubNumericDigits = 2;
		double dValue = (rand() % 110000 - 10000) / 100.0;
		pifFnd_SetFloat(s_pstFnd, dValue);

		pifLog_Printf(LT_enInfo, "Blink:%d Float:%d Value:%2f", swBlink, swFloat, dValue);
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
}

void appSetup()
{
    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifFnd_Init(g_pstTimer1ms, FND_COUNT)) return;
    pifFnd_SetUserChar(c_ucUserChar, 2);
    s_pstFnd = pifFnd_Add(PIF_ID_AUTO, 8, actFndDisplay);
    if (!s_pstFnd) return;
    pifFnd_SetControlPeriod(s_pstFnd, 16);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;					// 5%

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;				// 500ms
    if (!pifTask_AddPeriodMs(3000, _taskFndTest, NULL)) return;				// 3000ms

    pifFnd_Start(s_pstFnd);
}
