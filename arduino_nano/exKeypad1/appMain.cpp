#include "appMain.h"
#include "exKeypad1.h"

#include "pifKeypad.h"
#include "pifLog.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	3
#define TASK_COUNT              4


PIF_stPulse *g_pstTimer1ms = NULL;

const char c_cKeys[ROWS * COLS] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};


static void _evtKeypadPressed(char cChar)
{
	pifLog_Printf(LT_enInfo, "Keypad:Pressed %c", cChar);
}

static void _evtKeypadReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_enInfo, "Keypad:Released %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadLongReleased(char cChar, uint32_t unOnTime)
{
	pifLog_Printf(LT_enInfo, "Keypad:LongReleased %c T:%lu", cChar, unOnTime);
}

static void _evtKeypadDoublePressed(char cChar)
{
	pifLog_Printf(LT_enInfo, "Keypad:DoublePressed %c", cChar);
}

void appSetup()
{
	PIF_stComm *pstCommLog;
	PIF_stKeypad *pstKeypad;

	pif_Init(NULL);
	pifLog_Init();

	if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

	pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, pstCommLog, TRUE)) return;	// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    pstKeypad = pifKeypad_Init(PIF_ID_AUTO, ROWS, COLS, c_cKeys);
    if (!pstKeypad) return;
    if (!pifTask_AddPeriodMs(10, pifKeypad_Task, pstKeypad, TRUE)) return;	// 10ms
    pifKeypad_AttachAction(actKeypadAcquire);
    pstKeypad->evtPressed = _evtKeypadPressed;
    pstKeypad->evtReleased = _evtKeypadReleased;
    pstKeypad->evtLongReleased = _evtKeypadLongReleased;
    pstKeypad->evtDoublePressed = _evtKeypadDoublePressed;

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL, TRUE)) return;		// 500ms
}
