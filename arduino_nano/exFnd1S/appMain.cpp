#include "appMain.h"
#include "exFnd1S.h"

#include "pifFnd.h"
#include "pifLog.h"


#define COMM_COUNT         		1
#define FND_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              5


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stFnd *s_pstFnd = NULL;

const uint8_t c_ucUserChar[] = {
		0x77, /*  A  */	0x7C, /*  b  */ 0x39, /*  C  */ 0x5E, /*  d  */ 	// 0
		0x79, /*  E  */ 0x71, /*  F  */ 0x3D, /*  G  */ 0x76, /*  H  */ 	// 4
		0x30, /*  I  */ 0x1E, /*  J  */ 0x7A, /*  K  */ 0x38, /*  L  */ 	// 8
		0x55, /*  m  */ 0x54, /*  n  */ 0x5C, /*  o  */	0x73, /*  P  */ 	// 12
		0x67, /*  q  */ 0x50, /*  r  */ 0x6D, /*  S  */	0x78, /*  t  */ 	// 16
		0x3E, /*  U  */ 0x7E, /*  V  */ 0x6A, /*  W  */ 0x36, /*  X  */ 	// 20
		0x6E, /*  y  */ 0x49, /*  Z  */										// 24
};


static uint16_t _taskFndTest(PIF_stTask *pstTask)
{
	static int i = 0;
	static BOOL swBlink = FALSE;
	char buf[2];

	(void)pstTask;

	if (i < 10) pifFnd_SetInterger(s_pstFnd, i);
	else {
		buf[0] = 'A' + i - 10;
		buf[1] = 0;
		pifFnd_SetString(s_pstFnd, buf);
	}
	i = (i + 1) % 36;
	if (!i) {
		if (swBlink) {
		    pifFnd_BlinkOff(s_pstFnd);
		}
		else {
		    pifFnd_BlinkOn(s_pstFnd, 200);
		}
		swBlink ^= 1;
	}

	pifLog_Printf(LT_enInfo, "%d", i);
	return 0;
}

void appSetup()
{
	PIF_stComm *pstCommLog;

    pif_Init(NULL);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifFnd_Init(FND_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    pifFnd_SetUserChar(c_ucUserChar, 26);
    s_pstFnd = pifFnd_Add(PIF_ID_AUTO, 1, actFndDisplay);
    if (!s_pstFnd) return;

    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms
    if (!pifTask_AddRatio(5, pifFnd_taskAll, NULL)) return;					// 5%

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;				// 500ms
    if (!pifTask_AddPeriodMs(1000, _taskFndTest, NULL)) return;				// 1000ms

    pifFnd_Start(s_pstFnd);
}
