// Do not remove the include below
#include <DueTimer.h>

#include "exStepMotor.h"

#include "pifStepMotor.h"
#include "pifLog.h"
#include "pifPulse.h"
#include "pifTask.h"
#include "pifTerminal.h"


#define PIN_LED_L				13
#define PIN_STEP_MOTOR_1		30
#define PIN_STEP_MOTOR_2		32
#define PIN_STEP_MOTOR_3		34
#define PIN_STEP_MOTOR_4		36

#define COMM_COUNT				1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	2
#define PULSE_ITEM_COUNT    	20
#define TASK_COUNT              5

#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


static PIF_stComm *s_pstSerial = NULL;
static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stPulse *g_pstTimer200us = NULL;
static PIF_stStepMotor *s_pstMotor = NULL;

static int CmdStepMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdStepMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

typedef struct {
	int nMode;
	uint16_t usBreakTime;
	uint32_t unStepCount;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 1000, 200 };


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _taskTerminal(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
    	SerialUSB.print((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
		if (SerialUSB.available()) {
			rxData = SerialUSB.read();
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
    }
}

static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 1) {
		pifLog_Printf(LT_enNone, "\n  Method: %d", s_pstMotor->enMethod);
		pifLog_Printf(LT_enNone, "\n  Operation: %d", s_pstMotor->enOperation);
		pifLog_Printf(LT_enNone, "\n  Direction: %d", s_pstMotor->ucDirection);
		pifLog_Printf(LT_enNone, "\n  P/S: %u", s_pstMotor->usCurrentPps);
		pifLog_Printf(LT_enNone, "\n  R/M: %2f", pifStepMotor_GetRpm(s_pstMotor));
		pifLog_Printf(LT_enNone, "\n  Step Count: %d", s_stStepMotorTest.unStepCount);
		pifLog_Printf(LT_enNone, "\n  Break Time: %d", s_stStepMotorTest.usBreakTime);
		return PIF_TERM_CMD_NO_ERROR;
	}
	else if (argc > 2) {
		if (!strcmp(argv[1], "mt")) {
			if (s_pstMotor->enState == MS_enIdle) {
				int value = atoi(argv[2]);
				if (value >= 0 && value <= 1) {
					pifStepMotor_SetMethod(s_pstMotor, (PIF_enStepMotorMethod)value);
					return PIF_TERM_CMD_NO_ERROR;
				}
			}
		}
		else if (!strcmp(argv[1], "op")) {
			int value = atoi(argv[2]);
			if (value >= SMO_en4P_UP_1Phase && value <= SMO_en4P_UP_12Phase) {
				pifStepMotor_SetOperation(s_pstMotor, (PIF_enStepMotorOperation)value);
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "pps")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetPps(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "rpm")) {
			float value = atof(argv[2]);
			if (value > 0) {
				if (!pifStepMotor_SetRpm(s_pstMotor, value)) {
					pifLog_Printf(LT_enError, "\n  Invalid Parameter: %d", value);
				}
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "dir")) {
			int value = atoi(argv[2]);
			if (value == 0 || value == 1) {
				s_pstMotor->ucDirection = value;
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cnt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.unStepCount = value;
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[1], "cbt")) {
			int value = atoi(argv[2]);
			if (value > 0) {
				s_stStepMotorTest.usBreakTime = value;
				return PIF_TERM_CMD_NO_ERROR;
			}
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	else if (argc > 1) {
		if (!strcmp(argv[1], "str")) {
			s_stStepMotorTest.nMode = 1;
			pifStepMotor_Start(s_pstMotor, 0);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sts")) {
			s_stStepMotorTest.nMode = 2;
			pifStepMotor_Start(s_pstMotor, s_stStepMotorTest.unStepCount);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "sp")) {
			s_stStepMotorTest.nMode = 0;
			pifStepMotor_BreakRelease(s_pstMotor, s_stStepMotorTest.usBreakTime);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "rel")) {
			pifStepMotor_Release(s_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static void _actSetStep(uint16_t usPhase)
{
	digitalWrite(PIN_STEP_MOTOR_1, usPhase & 1);
	digitalWrite(PIN_STEP_MOTOR_2, (usPhase >> 1) & 1);
	digitalWrite(PIN_STEP_MOTOR_3, (usPhase >> 2) & 1);
	digitalWrite(PIN_STEP_MOTOR_4, (usPhase >> 3) & 1);
}

void _evtStop(PIF_stStepMotor *pstOwner, void *pvInfo)
{
	(void)pvInfo;

	s_stStepMotorTest.nMode = 0;
	pifStepMotor_BreakRelease(pstOwner, s_stStepMotorTest.usBreakTime);
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
	pifPulse_sigTick(g_pstTimer200us);
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
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT);
    if (!s_pstTimer1ms) return;
    g_pstTimer200us = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT);
    if (!g_pstTimer200us) return;

    if (!pifStepMotor_Init(g_pstTimer200us, 200, MOTOR_COUNT)) return;
    s_pstMotor = pifStepMotor_Add(PIF_ID_AUTO, STEP_MOTOR_RESOLUTION, SMO_en4P_UP_1Phase);
    if (!s_pstMotor) return;
    pifStepMotor_AttachAction(s_pstMotor, _actSetStep);
    pifStepMotor_AttachEvent(s_pstMotor, NULL, _evtStop, NULL);
	s_pstMotor->ucReductionGearRatio = STEP_MOTOR_REDUCTION_GEAR_RATIO;
	pifStepMotor_SetPps(s_pstMotor, 200);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodMs(10, pifComm_taskAll, NULL)) return;		// 10ms
    pstTask = pifTask_AddPeriodUs(200, pifStepMotor_taskAll, NULL);		// 200us
    if (!pstTask) return;
    pifStepMotor_AttachTask(s_pstMotor, pstTask);

    if (!pifTask_AddPeriodMs(10, _taskTerminal, NULL)) return;			// 10ms
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;		// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
