#include "appMain.h"
#include "exDutyMotorPos.h"

#include "pifDutyMotorPos.h"
#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"
#include "pifTerminal.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define SWITCH_COUNT         	3
#define TASK_COUNT              5


PIF_stComm *g_pstSerial = NULL;
PIF_stDutyMotor *g_pstMotor = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdDutyMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	4

const PIF_stDutyMotorPosStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_enCCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[0],
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_enCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[2],
				0, 0,
				50, 0,
				0, 0, 100,
				0
		},
		{
				MM_D_enCW | MM_PC_enYes | MM_CIAS_enYes,
				&s_pstSwitch[0], &s_pstSwitch[1], &s_pstSwitch[2],
				50, 50,
				200, 50000,
				40, 50, 500,
				70000
		},
		{
				MM_D_enCCW | MM_PC_enYes | MM_CIAS_enYes,
				&s_pstSwitch[2], &s_pstSwitch[1], &s_pstSwitch[0],
				50, 50,
				200, 50000,
				40, 50, 500,
				70000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, 0 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Stage: %d", s_stDutyMotorTest.ucStage);
		return PIF_TERM_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stDutyMotorTest.ucStage = 0;
				pifDutyMotorPos_Stop(g_pstMotor);
				return PIF_TERM_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorPos_Start(g_pstMotor, value - 1, 2000)) {
						s_stDutyMotorTest.ucStage = value;
					}
				}
				else {
					pifLog_Printf(LT_enNone, "\nError: Stage=%d", s_stDutyMotorTest.ucStage);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			pifLog_Printf(LT_enInfo, "Stop");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorPos_Stop(g_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_enInfo, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorPos_Emergency(g_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "init")) {
			pifLog_Printf(LT_enInfo, "Init Pos");
		    s_stDutyMotorTest.ucInitPos = 1;
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PIF_stDutyMotor *pstOwner)
{
	PIF_stDutyMotorPos *pstChild = (PIF_stDutyMotorPos *)pstOwner->_pvChild;

	pifLog_Printf(LT_enInfo, "EventStable(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstChild->_unCurrentPulse);
}

static void _evtStop(PIF_stDutyMotor *pstOwner)
{
	PIF_stDutyMotorPos *pstChild = (PIF_stDutyMotorPos *)pstOwner->_pvChild;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstChild->_unCurrentPulse);
}

static void _evtError(PIF_stDutyMotor *pstOwner)
{
	PIF_stDutyMotorPos *pstChild = (PIF_stDutyMotorPos *)pstOwner->_pvChild;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstChild->_unCurrentPulse);
}

static void _taskInitPos(PIF_stTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stDutyMotorTest.ucInitPos) {
	case 1:
		unTime = 200;
		s_stDutyMotorTest.ucStage = 0;
		s_stDutyMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_enInfo, "InitPos: Start");
		break;

	case 2:
		if (!s_stDutyMotorTest.ucStage) {
			if (s_pstSwitch[0]->_swCurrState == ON) {
				pifLog_Printf(LT_enInfo, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorPos_Start(g_pstMotor, 0, unTime)) {
					s_stDutyMotorTest.ucStage = 1;
					s_stDutyMotorTest.ucInitPos = 3;
					unTime += 200;
				}
				else {
					s_stDutyMotorTest.ucInitPos = 4;
				}
			}
		}
		break;

	case 3:
		if (!s_stDutyMotorTest.ucStage) {
			if (pifDutyMotorPos_Start(g_pstMotor, 1, unTime)) {
				s_stDutyMotorTest.ucStage = 2;
				s_stDutyMotorTest.ucInitPos = 2;
				unTime += 200;
			}
			else {
				s_stDutyMotorTest.ucInitPos = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_enError, "InitPos: Error");
		s_stDutyMotorTest.ucInitPos = 0;
		break;
	}
}

void appSetup()
{
	PIF_stLed *pstLedL = NULL;

    pifLog_Init();
	pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    g_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstSerial) return;

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(g_pstSerial);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    if (!pifLed_Init(g_pstTimer1ms, LED_COUNT)) return;
    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSensorSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    if (!pifDutyMotor_Init(g_pstTimer1ms, MOTOR_COUNT)) return;
    g_pstMotor = pifDutyMotorPos_Add(PIF_ID_AUTO, 255, 10);
    pifDutyMotorPos_AddStages(g_pstMotor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    pifDutyMotor_AttachAction(g_pstMotor, actSetDuty, actSetDirection, actOperateBreak);
    g_pstMotor->evtStable = _evtStable;
    g_pstMotor->evtStop = _evtStop;
    g_pstMotor->evtError = _evtError;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;				// 100%
    if (!pifTask_AddPeriodMs(1, pifComm_taskAll, NULL)) return;				// 1ms
    if (!pifTask_AddPeriodMs(1, pifSensorSwitch_taskAll, NULL)) return;		// 1ms

    if (!pifTask_AddPeriodMs(1, taskTerminal, NULL)) return;				// 1ms
    if (!pifTask_AddPeriodMs(10, _taskInitPos, NULL)) return;				// 10ms
}
