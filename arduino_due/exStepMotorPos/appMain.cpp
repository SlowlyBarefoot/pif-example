#include "appMain.h"
#include "exStepMotorPos.h"

#include "pifLed.h"
#include "pifLog.h"
#include "pifSensorSwitch.h"
#include "pifStepMotorPos.h"


#define COMM_COUNT				1
#define LED_COUNT         		1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	10
#define SWITCH_COUNT         	3
#define TASK_COUNT              10

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PIF_stPulse *g_pstTimer1ms = NULL;
PIF_stPulse *g_pstTimer200us = NULL;

static PIF_stStepMotor *s_pstMotor = NULL;
static PIF_stSensor *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stLogCmdEntry c_psCmdTable[] = {
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	5

const PIF_stStepMotorPosStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
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
				MM_D_enCW | MM_PC_enYes,
				NULL, NULL, NULL,
				200, 50,
				800, 750,
				200, 50, 100,
				1000
		},
		{
				MM_D_enCCW | MM_PC_enYes,
				NULL, NULL, NULL,
				200, 50,
				800, 750,
				200, 50, 100,
				1000
		},
		{
				MM_D_enCCW | MM_PC_enYes | MM_NR_enYes,
				NULL, NULL, NULL,
				100, 25,
				500, 800,
				100, 50, 100,
				1000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
    uint8_t ucRepeat;
    uint8_t ucRepeatStop;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 0, 0, 0 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Stage: %d", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_enNone, "\n  Method: %d", s_pstMotor->_enMethod);
		pifLog_Printf(LT_enNone, "\n  Operation: %d", s_pstMotor->_enOperation);
	    pifLog_Printf(LT_enNone, "\n  Timer 1ms: Max=%u Use=%u", g_pstTimer1ms->_ucItemSize, g_pstTimer1ms->_ucItemCount);
	    pifLog_Printf(LT_enNone, "\n  Timer 200us: Max=%u Use=%u", g_pstTimer200us->_ucItemSize, g_pstTimer200us->_ucItemCount);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "mt")) {
			if (s_pstMotor->_enState == MS_enIdle) {
				int value = atoi(argv[2]);
				if (value >= 0 && value <= 1) {
					pifStepMotor_SetMethod(s_pstMotor, (PIF_enStepMotorMethod)value);
					return PIF_LOG_CMD_NO_ERROR;
				}
			}
		}
		else if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_en2P_4W_1S && value <= SMO_en2P_4W_1_2S) {
				pifStepMotor_SetOperation(s_pstMotor, (PIF_enStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorPos_Stop(s_pstMotor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorPos_Start(s_pstMotor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_enNone, "\nError: Stage=%d", s_stStepMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "repeat")) {
			int value = atoi(argv[2]);
			switch (value) {
			case 0:
			    s_stStepMotorTest.ucRepeatStop = 1;
				return PIF_LOG_CMD_NO_ERROR;

			case 1:
			    s_stStepMotorTest.ucRepeat = 1;
			    s_stStepMotorTest.ucRepeatStop = 0;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Stop(s_pstMotor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorPos_Emergency(s_pstMotor);
		}
		else if (!strcmp(argv[1], "init")) {
			pifLog_Printf(LT_enInfo, "Init Pos");
		    s_stStepMotorTest.ucInitPos = 1;
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PIF_stStepMotor *pstOwner)
{
	PIF_stStepMotorPos *pstChild = (PIF_stStepMotorPos *)pstOwner->_pvChild;

	pifLog_Printf(LT_enInfo, "EventStable(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtStop(PIF_stStepMotor *pstOwner)
{
	PIF_stStepMotorPos *pstChild = (PIF_stStepMotorPos *)pstOwner->_pvChild;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtError(PIF_stStepMotor *pstOwner)
{
	PIF_stStepMotorPos *pstChild = (PIF_stStepMotorPos *)pstOwner->_pvChild;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d) : S=%u P=%u", pstOwner->_usPifId, pstChild->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static uint16_t _taskInitPos(PIF_stTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stStepMotorTest.ucInitPos) {
	case 1:
		unTime = 500;
		s_stStepMotorTest.ucStage = 0;
		s_stStepMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_enInfo, "InitPos: Start");
		break;

	case 2:
		if (!s_stStepMotorTest.ucStage) {
			if (s_pstSwitch[0]->_swCurrState == ON) {
				pifLog_Printf(LT_enInfo, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorPos_Start(s_pstMotor, 0, unTime)) {
					s_stStepMotorTest.ucStage = 1;
					s_stStepMotorTest.ucInitPos = 3;
					unTime += 500;
				}
				else {
					s_stStepMotorTest.ucInitPos = 4;
				}
			}
		}
		break;

	case 3:
		if (!s_stStepMotorTest.ucStage) {
			if (pifStepMotorPos_Start(s_pstMotor, 1, unTime)) {
				s_stStepMotorTest.ucStage = 2;
				s_stStepMotorTest.ucInitPos = 2;
				unTime += 500;
			}
			else {
				s_stStepMotorTest.ucInitPos = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_enError, "InitPos: Error");
		s_stStepMotorTest.ucInitPos = 0;
		break;
	}
	return 0;
}

static uint16_t _taskRepeat(PIF_stTask *pstTask)
{
	(void)pstTask;

	switch (s_stStepMotorTest.ucRepeat) {
	case 1:
		s_stStepMotorTest.ucStage = 0;
		s_stStepMotorTest.ucRepeat = 2;
		pifLog_Printf(LT_enInfo, "Repeat: Start");
		break;

	case 2:
		if (!s_stStepMotorTest.ucStage) {
			if (s_stStepMotorTest.ucRepeatStop) {
				s_stStepMotorTest.ucRepeat = 5;
			}
			else if (pifStepMotorPos_Start(s_pstMotor, 2, 0)) {
				s_stStepMotorTest.ucStage = 3;
				s_stStepMotorTest.ucRepeat = 3;
			}
			else {
				s_stStepMotorTest.ucRepeat = 4;
			}
		}
		break;

	case 3:
		if (!s_stStepMotorTest.ucStage) {
			if (s_stStepMotorTest.ucRepeatStop) {
				s_stStepMotorTest.ucRepeat = 5;
			}
			if (pifStepMotorPos_Start(s_pstMotor, 3, 0)) {
				s_stStepMotorTest.ucStage = 2;
				s_stStepMotorTest.ucRepeat = 2;
			}
			else {
				s_stStepMotorTest.ucRepeat = 4;
			}
		}
		break;

	case 4:
		pifLog_Printf(LT_enError, "Repeat: Error");
		s_stStepMotorTest.ucRepeat = 0;
		s_stStepMotorTest.ucRepeatStop = 0;
		break;

	case 5:
		pifLog_Printf(LT_enError, "Repeat: Stop");
		s_stStepMotorTest.ucRepeat = 0;
		s_stStepMotorTest.ucRepeatStop = 0;
		break;
	}
	return 0;
}

void appSetup(PIF_actTimer1us actTimer1us)
{
	PIF_stComm *pstCommLog;
	PIF_stLed *pstLedL;

	pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifSensorSwitch_Init(SWITCH_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);									// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;						// 100%

    g_pstTimer200us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 200);									// 200us
    if (!g_pstTimer200us) return;
    if (!pifTask_Add(TM_enRatio, 100, pifPulse_Task, g_pstTimer200us, TRUE)) return;					// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;
    if (!pifStepMotor_Init(MOTOR_COUNT, g_pstTimer200us)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_Add(TM_enPeriodMs, 1, pifComm_Task, pstCommLog, TRUE)) return;							// 1ms
	pifComm_AttachActReceiveData(pstCommLog, actLogReceiveData);
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug")) return;

    pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;														// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSensorSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_pstSwitch[i]) return;
	    if (!pifTask_Add(TM_enPeriodMs, 1, pifSensorSwitch_Task, s_pstSwitch[i], TRUE)) return;			// 1ms
	    pifSensor_AttachAction(s_pstSwitch[i], actPhotoInterruptAcquire);
    }

    s_pstMotor = pifStepMotorPos_Add(PIF_ID_AUTO, STEP_MOTOR_RESOLUTION, SMO_en2P_4W_1S, 100);			// 20ms
    if (!s_pstMotor) return;
    if (!pifTask_Add(TM_enPeriodUs, 200, pifStepMotor_Task, s_pstMotor, FALSE)) return;					// 200us
    pifStepMotor_AttachAction(s_pstMotor, actSetStep);
    s_pstMotor->evtStable = _evtStable;
    s_pstMotor->evtStop = _evtStop;
    s_pstMotor->evtError = _evtError;
    pifStepMotor_SetReductionGearRatio(s_pstMotor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
    pifStepMotorPos_AddStages(s_pstMotor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    if (!pifTask_Add(TM_enPeriodMs, 20, pifLog_Task, NULL, TRUE)) return;								// 20ms

    if (!pifTask_Add(TM_enPeriodMs, 10, _taskInitPos, NULL, TRUE)) return;								// 10ms
    if (!pifTask_Add(TM_enPeriodMs, 10, _taskRepeat, NULL, TRUE)) return;								// 10ms
}
