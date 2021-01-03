// Do not remove the include below
#include "exDutyMotorSpeed.h"

#include "pifDutyMotorSpeed.h"
#include "pifLog.h"
#include "pifPulse.h"
#include "pifTask.h"
#include "pifTerminal.h"


#define PIN_LED_L				13
#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN1			30
#define PIN_L298N_IN2			32

#define COMM_COUNT				1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	20
#define TASK_COUNT              4


static PIF_stPulse *s_pstTimer1ms = NULL;

static PIF_stComm *s_pstSerial = NULL;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	5

const PIF_stDutyMotorSpeedStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_enDefault,
				48, 16,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_enCCW | MM_NR_enNo,
				64, 16,
				200, 500,
				32, 16, 1000
		},
		{
				MM_enDefault,
				0, 0,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_enCCW | MM_NR_enNo,
				64, 16,
				200, 0,
				0, 0, 0
		},
		{
				MM_enDefault,
				64, 16,
				200, 0,
				0, 0, 2000
		}
};

typedef struct {
	uint8_t ucStage;
    PIF_stDutyMotor *pstMotor;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, NULL };


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
				pifDutyMotorSpeed_Stop(s_stDutyMotorTest.pstMotor);
				return PIF_TERM_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					s_stDutyMotorTest.ucStage = value;
					pifDutyMotorSpeed_Start(s_stDutyMotorTest.pstMotor, s_stDutyMotorTest.ucStage - 1);
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
			pifDutyMotorSpeed_Stop(s_stDutyMotorTest.pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_enInfo, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Emergency(s_stDutyMotorTest.pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		return PIF_TERM_CMD_INVALID_ARG;
	}
	return PIF_TERM_CMD_TOO_FEW_ARGS;
}

static void _actSetDuty(uint16_t usDuty)
{
	analogWrite(PIN_L298N_ENB_PWM, usDuty);
	pifLog_Printf(LT_enInfo, "SetDuty(%d)", usDuty);
}

static void _actSetDirection(uint8_t ucDir)
{
	if (ucDir) {
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, HIGH);
	}
	else {
		digitalWrite(PIN_L298N_IN1, HIGH);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	pifLog_Printf(LT_enInfo, "SetDirection(%d)", ucDir);
}

static void _actOperateBreak(uint8_t ucState)
{
	if (ucState) {
		analogWrite(PIN_L298N_ENB_PWM, 255);
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	else {
		analogWrite(PIN_L298N_ENB_PWM, 0);
	}
	pifLog_Printf(LT_enInfo, "OperateBreak(%d)", ucState);
}

static void _evtStable(PIF_stDutyMotor *pstParent, void *pvInfo)
{
	(void)pvInfo;

	pifLog_Printf(LT_enInfo, "EventStable(%d)", pstParent->unDeviceCode);
}

static void _evtStop(PIF_stDutyMotor *pstParent, void *pvInfo)
{
	(void)pvInfo;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d)", pstParent->unDeviceCode);
}

static void _evtError(PIF_stDutyMotor *pstParent, void *pvInfo)
{
	(void)pvInfo;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d)", pstParent->unDeviceCode);
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

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unDeviceCode unDeviceCode = 1;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN1, OUTPUT);
	pinMode(PIN_L298N_IN2, OUTPUT);

	Serial.begin(115200);
	SerialUSB.begin(115200);

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;
    s_pstSerial = pifComm_Add(unDeviceCode++);
	if (!s_pstSerial) return;

    if (!pifTerminal_Init(c_psCmdTable, "\nDebug")) return;
	pifTerminal_AttachComm(s_pstSerial);

	pifLog_DetachActPrint();
    pifLog_UseTerminal(TRUE);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer1ms) return;

    if (!pifDutyMotor_Init(s_pstTimer1ms, MOTOR_COUNT)) return;
    s_stDutyMotorTest.pstMotor = pifDutyMotorSpeed_Add(unDeviceCode++, 255, 100);
    pifDutyMotorSpeed_AddStages(s_stDutyMotorTest.pstMotor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    pifDutyMotor_AttachAction(s_stDutyMotorTest.pstMotor, _actSetDuty, _actSetDirection, _actOperateBreak);
    pifDutyMotor_AttachEvent(s_stDutyMotorTest.pstMotor, _evtStable, _evtStop, _evtError);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodMs(5, pifComm_taskAll, NULL)) return;		// 5ms

    if (!pifTask_AddPeriodMs(5, _taskTerminal, NULL)) return;		// 5ms
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
