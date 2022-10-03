#include "appMain.h"
#include "exFnd1S.h"

#include "core/pif_log.h"
#include "display/pif_fnd.h"


PifTimerManager g_timer_1ms;

static PifFnd s_fnd;

const uint8_t c_ucUserChar[] = {
		0x77, /*  A  */	0x7C, /*  b  */ 0x39, /*  C  */ 0x5E, /*  d  */ 	// 0
		0x79, /*  E  */ 0x71, /*  F  */ 0x3D, /*  G  */ 0x76, /*  H  */ 	// 4
		0x30, /*  I  */ 0x1E, /*  J  */ 0x7A, /*  K  */ 0x38, /*  L  */ 	// 8
		0x55, /*  m  */ 0x54, /*  n  */ 0x5C, /*  o  */	0x73, /*  P  */ 	// 12
		0x67, /*  q  */ 0x50, /*  r  */ 0x6D, /*  S  */	0x78, /*  t  */ 	// 16
		0x3E, /*  U  */ 0x7E, /*  V  */ 0x6A, /*  W  */ 0x36, /*  X  */ 	// 20
		0x6E, /*  y  */ 0x49, /*  Z  */										// 24
};


static uint16_t _taskFndTest(PifTask *pstTask)
{
	static int i = 0;
	static BOOL swBlink = FALSE;
	char buf[2];

	(void)pstTask;

	if (i < 10) pifFnd_SetInterger(&s_fnd, i);
	else {
		buf[0] = 'A' + i - 10;
		buf[1] = 0;
		pifFnd_SetString(&s_fnd, buf);
	}
	i = (i + 1) % 36;
	if (!i) {
		if (swBlink) {
		    pifFnd_BlinkOff(&s_fnd);
		}
		else {
		    pifFnd_BlinkOn(&s_fnd, 200);
		}
		swBlink ^= 1;
	}

	pifLog_Printf(LT_INFO, "%d", i);
	return 0;
}

void appSetup()
{
	static PifComm s_comm_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    pifFnd_SetUserChar(c_ucUserChar, 26);
    if (!pifFnd_Init(&s_fnd, PIF_ID_AUTO, &g_timer_1ms, 1, actFndDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;	// 500ms
    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskFndTest, NULL, TRUE)) return;	// 1000ms

    pifFnd_Start(&s_fnd);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
