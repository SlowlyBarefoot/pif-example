#include "appMain.h"


PifFnd g_fnd;
PifTimerManager g_timer_1ms;

const uint8_t c_ucUserChar[] = { 0x01, 0x08 };


static uint16_t _taskFndTest(PifTask *pstTask)
{
	static int nBlink = 0;
	static BOOL swBlink = FALSE;
	static BOOL swFloat = FALSE;
	static int nRun = 0;
	static BOOL swRun = FALSE;

	(void)pstTask;

	if (swRun) {
		nRun++;
		if (nRun >= 30) {
			pifFnd_Stop(&g_fnd);
			swRun = FALSE;
			nRun = 0;
		}
	}
	else {
		nRun++;
		if (nRun >= 3) {
			pifFnd_Start(&g_fnd);
			swRun = TRUE;
			nRun = 0;
		}
	}
	if (swFloat) {
		g_fnd.sub_numeric_digits = 0;
		int32_t nValue = rand() % 14000 - 2000;
		if (nValue <= -1000) {
			pifFnd_SetString(&g_fnd, (char *)"AAAA");
		}
		else if (nValue < 10000) {
			pifFnd_SetInterger(&g_fnd, nValue);
		}
		else {
			pifFnd_SetString(&g_fnd, (char *)"BBBB");
		}

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%d", swBlink, swFloat, nValue);
	}
	else {
		g_fnd.sub_numeric_digits = 1;
		double dValue = (rand() % 11000 - 1000) / 10.0;
		pifFnd_SetFloat(&g_fnd, dValue);

		pifLog_Printf(LT_INFO, "Blink:%d Float:%d Value:%1f", swBlink, swFloat, dValue);
	}
	swFloat ^= 1;
	nBlink = (nBlink + 1) % 20;
	if (!nBlink) {
		if (swBlink) {
		    pifFnd_BlinkOff(&g_fnd);
		}
		else {
		    pifFnd_BlinkOn(&g_fnd, 200);
		}
		swBlink ^= 1;
	}
	return 0;
}

BOOL appSetup()
{
    pifFnd_SetUserChar(c_ucUserChar, 2);
    pifFnd_SetPeriodPerDigit(&g_fnd, 20);													// 20ms

	if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskFndTest, NULL, TRUE)) return FALSE;	// 1000ms
	return TRUE;
}
