#include "appMain.h"
#include "exStepMotor.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "motor/pif_step_motor.h"


#define STEP_MOTOR_RESOLUTION				200
#define STEP_MOTOR_REDUCTION_GEAR_RATIO		1


PifTimerManager g_timer_1ms;
PifTimerManager g_timer_200us;

static PifStepMotor s_motor;

static int CmdStepMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdStepMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

typedef struct {
	int nMode;
	uint16_t usBreakTime;
	uint32_t unStepCount;
} ST_StepMotorTest;

static ST_StepMotorTest s_stStepMotorTest = { 0, 1000, 200 };


static int CmdStepMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Operation: %d\n", s_motor._operation);
		pifLog_Printf(LT_NONE, "  Direction: %d\n", s_motor._direction);
		pifLog_Printf(LT_NONE, "  P/S: %u\n", s_motor._current_pps);
		pifLog_Printf(LT_NONE, "  R/M: %2f\n", pifStepMotor_GetRpm(&s_motor));
		pifLog_Printf(LT_NONE, "  Step Count: %d\n", s_stStepMotorTest.unStepCount);
		pifLog_Printf(LT_NONE, "  Break Time: %d\n", s_stStepMotorTest.usBreakTime);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "op")) {
			int value = atoi(argv[1]);
			if (value >= SMO_2P_4W_1S && value <= SMO_2P_4W_1_2S) {
				pifStepMotor_SetOperation(&s_motor, (PifStepMotorOperation)value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "pps")) {
			float value = atof(argv[1]);
			if (value > 0) {
				if (!pifStepMotor_SetPps(&s_motor, value)) {
					pifLog_Printf(LT_ERROR, "  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "rpm")) {
			float value = atof(argv[1]);
			if (value > 0) {
				if (!pifStepMotor_SetRpm(&s_motor, value)) {
					pifLog_Printf(LT_ERROR, "  Invalid Parameter: %d", value);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "dir")) {
			int value = atoi(argv[1]);
			if (value == 0 || value == 1) {
				pifStepMotor_SetDirection(&s_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "cnt")) {
			int value = atoi(argv[1]);
			if (value > 0) {
				s_stStepMotorTest.unStepCount = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "cbt")) {
			int value = atoi(argv[1]);
			if (value > 0) {
				s_stStepMotorTest.usBreakTime = value;
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "str")) {
			s_stStepMotorTest.nMode = 1;
			pifStepMotor_Start(&s_motor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "sts")) {
			s_stStepMotorTest.nMode = 2;
			pifStepMotor_Start(&s_motor, s_stStepMotorTest.unStepCount);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "sp")) {
			s_stStepMotorTest.nMode = 0;
			pifStepMotor_BreakRelease(&s_motor, s_stStepMotorTest.usBreakTime);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "rel")) {
			pifStepMotor_Release(&s_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStop(PifStepMotor *pstOwner)
{
	s_stStepMotorTest.nMode = 0;
	pifStepMotor_BreakRelease(pstOwner, s_stStepMotorTest.usBreakTime);
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	static PifLed s_led_l;

	pif_Init(act_timer1us);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;		// 1000us

    if (!pifTimerManager_Init(&g_timer_200us, PIF_ID_AUTO, 200, 1)) return;		// 200us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
	s_comm_log.act_receive_data = actLogReceiveData;
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;							// 500ms

    if (!pifStepMotor_Init(&s_motor, PIF_ID_AUTO, &g_timer_200us, STEP_MOTOR_RESOLUTION, SMO_2P_4W_2S)) return;
    s_motor.act_set_step = actSetStep;
    s_motor.evt_stop = _evtStop;
    pifStepMotor_SetReductionGearRatio(&s_motor, STEP_MOTOR_REDUCTION_GEAR_RATIO);
	pifStepMotor_SetPps(&s_motor, 200);

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer 1ms=%d Timer200us=%d\n", pifTaskManager_Count(),
			pifTimerManager_Count(&g_timer_1ms), pifTimerManager_Count(&g_timer_200us));
}
