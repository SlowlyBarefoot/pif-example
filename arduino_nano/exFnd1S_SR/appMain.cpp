#include "appMain.h"


PifFnd g_fnd;
PifTimerManager g_timer_1ms;

const uint8_t c_ucUserChar[] = {
		0x77, /*  A  */	0x7C, /*  b  */ 0x39, /*  C  */ 0x5E, /*  d  */ 	// 0
		0x79, /*  E  */ 0x71, /*  F  */ 0x3D, /*  G  */ 0x76, /*  H  */ 	// 4
		0x30, /*  I  */ 0x1E, /*  J  */ 0x7A, /*  K  */ 0x38, /*  L  */ 	// 8
		0x55, /*  m  */ 0x54, /*  n  */ 0x5C, /*  o  */	0x73, /*  P  */ 	// 12
		0x67, /*  q  */ 0x50, /*  r  */ 0x6D, /*  S  */	0x78, /*  t  */ 	// 16
		0x3E, /*  U  */ 0x7E, /*  V  */ 0x6A, /*  W  */ 0x36, /*  X  */ 	// 20
		0x6E, /*  y  */ 0x49, /*  Z  */										// 24
};


static uint32_t _taskFndTest(PifTask *pstTask)
{
	static int i = 0;
	static BOOL swBlink = FALSE;
	char buf[2];

	(void)pstTask;

	if (i < 10) pifFnd_SetInterger(&g_fnd, i);
	else {
		buf[0] = 'A' + i - 10;
		buf[1] = 0;
		pifFnd_SetString(&g_fnd, buf);
	}
	i = (i + 1) % 36;
	if (!i) {
		if (swBlink) {
		    pifFnd_BlinkOff(&g_fnd);
		}
		else {
		    pifFnd_BlinkOn(&g_fnd, 200);
		}
		swBlink ^= 1;
	}

	pifLog_Printf(LT_INFO, "%d", i);
	return 0;
}

BOOL appSetup()
{
    pifFnd_SetUserChar(c_ucUserChar, 26);

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000000, _taskFndTest, NULL, TRUE)) return FALSE;	// 1000ms

    pifFnd_Start(&g_fnd);
    return TRUE;
}
