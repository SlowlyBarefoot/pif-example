#include "appMain.h"
#include "exDutyMotor.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "motor/pif_duty_motor.h"


PifTimerManager g_timer_1ms;

static PifDutyMotor s_motor;

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdDutyMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

typedef struct {
	uint16_t usDuty;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 128 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Duty: %d\n", s_motor._current_duty);
		pifLog_Printf(LT_NONE, "  Direction: %d\n", s_motor._direction);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "duty")) {
			int value = atoi(argv[1]);
			if (value > 0 && value < 256) {
				pifDutyMotor_SetDuty(&s_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		else if (!strcmp(argv[0], "dir")) {
			int value = atoi(argv[1]);
			if (value == 0 || value == 1) {
				pifDutyMotor_SetDirection(&s_motor, value);
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "stop")) {
			pifDutyMotor_BreakRelease(&s_motor, 0);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "break")) {
			pifDutyMotor_BreakRelease(&s_motor, 1000);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "start")) {
			pifDutyMotor_Start(&s_motor, s_stDutyMotorTest.usDuty);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

void appSetup()
{
	static PifUart s_uart_log;
	static PifLed s_led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

	pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;		// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;							// 500ms

    if (!pifDutyMotor_Init(&s_motor, PIF_ID_AUTO, &g_timer_1ms, 255)) return;
    s_motor.act_set_duty = actSetDuty;
    s_motor.act_set_direction = actSetDirection;
    s_motor.act_operate_break = actOperateBreak;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
