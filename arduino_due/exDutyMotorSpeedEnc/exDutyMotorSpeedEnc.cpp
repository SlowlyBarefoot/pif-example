// Do not remove the include below
#include "exDutyMotorSpeedEnc.h"

#include "pifDutyMotorSpeedEnc.h"
#include "pifLog.h"
#include "pifPulse.h"
#include "pifTask.h"
#include "pifTerminal.h"


#define PIN_LED_L				13
#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN1			30
#define PIN_L298N_IN2			32
#define PIN_ENCODER				34
#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39

#define COMM_COUNT				1
#define MOTOR_COUNT				1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	30
#define SWITCH_COUNT         	3
#define TASK_COUNT              6

#define PIF_ID_SWITCH(n)		(0x100 + (n))


static PIF_stComm *s_pstSerial = NULL;
static PIF_stDutyMotor *s_pstMotor = NULL;
static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stSwitch *s_pstSwitch[SWITCH_COUNT] = { NULL, NULL, NULL };

static int CmdDutyMotorTest(int argc, char *argv[]);

const PIF_stTermCmdEntry c_psCmdTable[] = {
	{ "ver", pifTerminal_PrintVersion, "\nPrint Version" },
	{ "status", pifTerminal_SetStatus, "\nSet Status" },
	{ "mt", CmdDutyMotorTest, "\nMotor Test" },

	{ NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	4

const PIF_stDutyMotorSpeedEncStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_enCCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[0],
				0, 0, 0, 0,
				500, 50, 0, 0, 90, 110,
				0, 0, 100
		},
		{
				MM_D_enCW | MM_RT_enTime | MM_CFPS_enYes,
				NULL, NULL, &s_pstSwitch[2],
				0, 0, 0, 0,
				500, 50, 0, 0, 90, 110,
				0, 0, 100
		},
		{
				MM_D_enCW | MM_SC_enYES | MM_CIAS_enYes | MM_CFPS_enYes,
				&s_pstSwitch[0], &s_pstSwitch[1], &s_pstSwitch[2],
				95, 48, 16, 5000,
				2300, 230, 0, 3000, 90, 110,
				50, 16, 1000
		},
		{
				MM_D_enCCW | MM_SC_enYES | MM_CIAS_enYes | MM_CFPS_enYes,
				&s_pstSwitch[2], &s_pstSwitch[1], &s_pstSwitch[0],
				95, 48, 16, 5000,
				2300, 230, 0, 3000, 90, 110,
				50, 16, 0
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, 0 };


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
				pifDutyMotorSpeedEnc_Stop(s_pstMotor);
				return PIF_TERM_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorSpeedEnc_Start(s_pstMotor, value - 1, 1000)) {
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
			pifDutyMotorSpeedEnc_Stop(s_pstMotor);
			return PIF_TERM_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[1], "em")) {
			pifLog_Printf(LT_enInfo, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeedEnc_Emergency(s_pstMotor);
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

	pifLog_Printf(LT_enInfo, "EventStable(%d)", pstParent->usPifId);
}

static void _evtStop(PIF_stDutyMotor *pstParent, void *pvInfo)
{
	(void)pvInfo;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventStop(%d)", pstParent->usPifId);
}

static void _evtError(PIF_stDutyMotor *pstParent, void *pvInfo)
{
	(void)pvInfo;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_enInfo, "EventError(%d)", pstParent->usPifId);
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
			if (s_pstSwitch[0]->swCurrState == ON) {
				pifLog_Printf(LT_enInfo, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorSpeedEnc_Start(s_pstMotor, 0, unTime)) {
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
			if (pifDutyMotorSpeedEnc_Start(s_pstMotor, 1, unTime)) {
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

static void _isrEncoder()
{
	if (s_pstMotor) {
		pifDutyMotorSpeedEnc_sigEncoder(s_pstMotor);
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

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN1, OUTPUT);
	pinMode(PIN_L298N_IN2, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

	pinMode(PIN_ENCODER, INPUT_PULLUP);
	attachInterrupt(PIN_ENCODER, _isrEncoder, FALLING);

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

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    for (int i = 0; i < SWITCH_COUNT; i++) {
		s_pstSwitch[i] = pifSwitch_Add(PIF_ID_SWITCH(i), 0);
		if (!s_pstSwitch[i]) return;
	    pifSwitch_AttachAction(s_pstSwitch[i], _PhotoInterruptAcquire);
    }

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT);
    if (!s_pstTimer1ms) return;

    if (!pifDutyMotor_Init(s_pstTimer1ms, MOTOR_COUNT)) return;
    s_pstMotor = pifDutyMotorSpeedEnc_Add(PIF_ID_AUTO, 255, 100);
    pifDutyMotorSpeedEnc_AddStages(s_pstMotor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    pifDutyMotor_AttachAction(s_pstMotor, _actSetDuty, _actSetDirection, _actOperateBreak);
    pifDutyMotor_AttachEvent(s_pstMotor, _evtStable, _evtStop, _evtError);
    pifPidControl_Init(pifDutyMotorSpeedEnc_GetPidControl(s_pstMotor), 0.1, 0, 0, 100);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodMs(5, pifComm_taskAll, NULL)) return;		// 5ms
    if (!pifTask_AddPeriodMs(1, pifSwitch_taskAll, NULL)) return;	// 1ms

    if (!pifTask_AddPeriodMs(5, _taskTerminal, NULL)) return;		// 5ms
    if (!pifTask_AddPeriodMs(10, _taskInitPos, NULL)) return;		// 10ms
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
