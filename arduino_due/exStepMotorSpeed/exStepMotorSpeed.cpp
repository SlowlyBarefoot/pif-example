// Do not remove the include below
#include <DueTimer.h>

#include "exStepMotorSpeed.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifStepMotorSpeed.h"
#include "pifTask.h"
#include "pifTerminal.h"


#define PIN_LED_L				13
#define PIN_STEP_MOTOR_1		30
#define PIN_STEP_MOTOR_2		32
#define PIN_STEP_MOTOR_3		34
#define PIN_STEP_MOTOR_4		36
#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39

#define COMM_COUNT				1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	20
#define SWITCH_COUNT         	3
#define TASK_COUNT              7

#define PIF_ID_SWITCH(n)		(0x100 + (n))

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


static PIF_stComm *s_pstSerial = NULL;
static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stPulse *s_pstTimer200us = NULL;
static PIF_stStepMotor *s_pstMotor = NULL;
static PIF_stSwitch *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define STEP_MOTOR_STAGE_COUNT	4

const PIF_stStepMotorSpeedStage s_stStepMotorStages[STEP_MOTOR_STAGE_COUNT] = {
		{
				MM_D_enCCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[0],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_enCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[2],
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_enCW,
				NULL, &s_pstSwitch[1], NULL,
				100, 25,
				500, 0,
				100, 50, 100
		},
		{
				MM_D_enCCW,
				NULL, &s_pstSwitch[1], NULL,
				100, 50,
				500, 0,
				100, 25, 100
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 0 };


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _taskTerminal(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(s_pstSerial, &txData)) {
    	SerialUSB.print((char)txData);
    }

    if (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
		if (SerialUSB.available()) {
			rxData = SerialUSB.read();
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
    }
}

static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Stage: %d", s_stStepMotorTest.ucStage);
		pifLog_Printf(LT_enNone, "\n  Method: %d", s_pstMotor->_enMethod);
		pifLog_Printf(LT_enNone, "\n  Operation: %d", s_pstMotor->_enOperation);
		return PIF_TERM_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "mt")) {
			if (s_pstMotor->_enState == MS_enIdle) {
				int value = atoi(argv[2]);
				if (value >= 0 && value <= 1) {
					pifStepMotor_SetMethod(s_pstMotor, (PIF_enStepMotorMethod)value);
					return PIF_TERM_CMD_NO_ERROR;
				}
			}
		}
		else if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_en2P_4W_1S && value <= SMO_en2P_4W_1_2S) {
				pifStepMotor_SetOperation(s_pstMotor, (PIF_enStepMotorOperation)value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "stage")) {
			int value = atoi(argv[2]);
			if (!value) {
				s_stStepMotorTest.ucStage = 0;
				pifStepMotorSpeed_Stop(s_pstMotor);
				return PIF_TERM_CMD_NO_ERROR;
			}
			else if (value <= STEP_MOTOR_STAGE_COUNT) {
				if (!s_stStepMotorTest.ucStage) {
					s_stStepMotorTest.ucStage = value;
					pifStepMotorSpeed_Start(s_pstMotor, s_stStepMotorTest.ucStage - 1, 2000);
				}
				else {
					pifLog_Printf(LT_enNone, "\nError: Stage=%d", s_stStepMotorTest.ucStage);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "off")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Stop(s_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			s_stStepMotorTest.ucStage = 0;
			pifStepMotorSpeed_Emergency(s_pstMotor);
		}
		else if (!strcmp(argv[1], "init")) {
			pifLog_Printf(LT_enInfo, "Init Pos");
		    s_stStepMotorTest.ucInitPos = 1;
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static SWITCH _PhotoInterruptAcquire(PIF_usId usPifId)
{
	switch (usPifId) {
	case PIF_ID_SWITCH(0):
		return digitalRead(PIN_PHOTO_INTERRUPT_1);

	case PIF_ID_SWITCH(1):
		return digitalRead(PIN_PHOTO_INTERRUPT_2);

	case PIF_ID_SWITCH(2):
		return digitalRead(PIN_PHOTO_INTERRUPT_3);
	}
	return OFF;
}

static void _actSetStep(uint16_t usPhase)
{
	digitalWrite(PIN_STEP_MOTOR_1, usPhase & 1);
	digitalWrite(PIN_STEP_MOTOR_2, (usPhase >> 1) & 1);
	digitalWrite(PIN_STEP_MOTOR_3, (usPhase >> 2) & 1);
	digitalWrite(PIN_STEP_MOTOR_4, (usPhase >> 3) & 1);
}

static void _evtStable(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	pifLog_Printf(LT_enInfo, "EventStable(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtStop(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _evtError(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	PIF_stStepMotorSpeed *pstInfo = (PIF_stStepMotorSpeed *)pvInfo;

	s_stStepMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d) : S=%u P=%u", pstOwner->_usPifId, pstInfo->_ucStageIndex, pstOwner->_unCurrentPulse);
}

static void _taskInitPos(PIF_stTask *pstTask)
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
			if (s_pstSwitch[0]->swCurrState == ON) {
				pifLog_Printf(LT_enInfo, "InitPos: Find");
				s_stStepMotorTest.ucInitPos = 0;
			}
			else {
				if (pifStepMotorSpeed_Start(s_pstMotor, 0, unTime)) {
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
			if (pifStepMotorSpeed_Start(s_pstMotor, 1, unTime)) {
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
}

static void _taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer1ms);
	}
}

static void _sigTimer200us()
{
	pifPulse_sigTick(s_pstTimer200us);
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_stTask *pstTask;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_STEP_MOTOR_1, OUTPUT);
	pinMode(PIN_STEP_MOTOR_2, OUTPUT);
	pinMode(PIN_STEP_MOTOR_3, OUTPUT);
	pinMode(PIN_STEP_MOTOR_4, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

	Timer3.attachInterrupt(_sigTimer200us).start(200);

	Serial.begin(115200);
	SerialUSB.begin(115200);

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(s_pstSerial);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;
    s_pstTimer200us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 200);		// 200us
    if (!s_pstTimer200us) return;

    if (!pifSwitch_Init(SWITCH_COUNT)) return;
    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSwitch_Add(PIF_ID_SWITCH(i), 0);
		if (!s_pstSwitch[i]) return;
	    pifSwitch_AttachAction(s_pstSwitch[i], _PhotoInterruptAcquire);
    }

    if (!pifStepMotor_Init(s_pstTimer200us, MOTOR_COUNT)) return;
    s_pstMotor = pifStepMotorSpeed_Add(PIF_ID_AUTO, STEP_MOTOR_RESOLUTION, SMO_en2P_4W_1S, 100);	// 100ms
    if (!s_pstMotor) return;
    pifStepMotor_AttachAction(s_pstMotor, _actSetStep);
    pifStepMotor_AttachEvent(s_pstMotor, _evtStable, _evtStop, _evtError);
	s_pstMotor->ucReductionGearRatio = STEP_MOTOR_REDUCTION_GEAR_RATIO;
    pifStepMotorSpeed_AddStages(s_pstMotor, STEP_MOTOR_STAGE_COUNT, s_stStepMotorStages);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodMs(10, pifComm_taskAll, NULL)) return;		// 10ms
    if (!pifTask_AddPeriodMs(1, pifSwitch_taskAll, NULL)) return;		// 1ms
    pstTask = pifTask_AddPeriodUs(200, pifStepMotor_taskAll, NULL);		// 200us
    if (!pstTask) return;
    pifStepMotor_AttachTask(s_pstMotor, pstTask);

    if (!pifTask_AddPeriodMs(10, _taskTerminal, NULL)) return;			// 10ms
    if (!pifTask_AddPeriodMs(10, _taskInitPos, NULL)) return;			// 10ms
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;		// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
