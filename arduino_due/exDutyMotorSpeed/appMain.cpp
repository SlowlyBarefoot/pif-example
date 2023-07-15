#include "appMain.h"
#include "exDutyMotorSpeed.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_sensor_switch.h"


#define SWITCH_COUNT         	3


PifDutyMotorSpeed g_motor;
PifTimerManager g_timer_1ms;

static PifSensorSwitch s_switch[SWITCH_COUNT];

static int CmdDutyMotorTest(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "mt", CmdDutyMotorTest, "Motor Test", NULL },

	{ NULL, NULL, NULL, NULL }
};

#define DUTY_MOTOR_STAGE_COUNT	7

const PifDutyMotorSpeedStage s_stDutyMotorStages[DUTY_MOTOR_STAGE_COUNT] = {
		{
				MM_D_CCW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[0].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_RT_TIME | MM_CFPS_YES,
				NULL, NULL, &s_switch[2].parent,
				0, 0,
				50, 0,
				0, 0, 100
		},
		{
				MM_D_CW | MM_CIAS_YES | MM_CFPS_YES,
				&s_switch[0].parent, &s_switch[1].parent, &s_switch[2].parent,
				48, 16,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_CCW | MM_CIAS_YES | MM_CFPS_YES,
				&s_switch[2].parent, &s_switch[1].parent, &s_switch[0].parent,
				64, 16,
				200, 500,
				32, 16, 1000
		},
		{
				MM_D_CW,
				NULL, NULL, NULL,
				0, 0,
				200, 0,
				32, 16, 1000
		},
		{
				MM_D_CCW,
				NULL, NULL, NULL,
				64, 16,
				200, 0,
				0, 0, 0
		},
		{
				MM_D_CW,
				NULL, NULL, NULL,
				64, 16,
				200, 0,
				0, 0, 2000
		}
};

typedef struct {
	uint8_t ucStage;
    uint8_t ucInitPos;
} ST_DutyMotorTest;

static ST_DutyMotorTest s_stDutyMotorTest = { 0, 0 };


static int CmdDutyMotorTest(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Stage: %d\n", s_stDutyMotorTest.ucStage);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 1) {
		if (!strcmp(argv[0], "stage")) {
			int value = atoi(argv[1]);
			if (!value) {
				s_stDutyMotorTest.ucStage = 0;
				pifDutyMotorSpeed_Stop(&g_motor);
				return PIF_LOG_CMD_NO_ERROR;
			}
			else if (value <= DUTY_MOTOR_STAGE_COUNT) {
				if (!s_stDutyMotorTest.ucStage) {
					if (pifDutyMotorSpeed_Start(&g_motor, value - 1, 2000)) {
						s_stDutyMotorTest.ucStage = value;
					}
				}
				else {
					pifLog_Printf(LT_NONE, "Error: Stage=%d\n", s_stDutyMotorTest.ucStage);
				}
				return PIF_LOG_CMD_NO_ERROR;
			}
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "off")) {
			pifLog_Printf(LT_INFO, "Stop");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Stop(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "em")) {
			pifLog_Printf(LT_INFO, "Emergency");
			s_stDutyMotorTest.ucStage = 0;
			pifDutyMotorSpeed_Emergency(&g_motor);
			return PIF_LOG_CMD_NO_ERROR;
		}
		else if (!strcmp(argv[0], "init")) {
			pifLog_Printf(LT_INFO, "Init Pos");
		    s_stDutyMotorTest.ucInitPos = 1;
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtStable(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	pifLog_Printf(LT_INFO, "EventStable(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static void _evtStop(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventStop(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static void _evtError(PifDutyMotor *pstOwner)
{
	PifDutyMotorSpeed* pstChild = (PifDutyMotorSpeed*)pstOwner;

	s_stDutyMotorTest.ucStage = 0;
	pifLog_Printf(LT_INFO, "EventError(%d) : S=%u", pstOwner->_id, pstChild->_stage_index);
}

static uint16_t _taskInitPos(PifTask *pstTask)
{
	static uint32_t unTime;

	(void)pstTask;

	switch (s_stDutyMotorTest.ucInitPos) {
	case 1:
		unTime = 200;
		s_stDutyMotorTest.ucStage = 0;
		s_stDutyMotorTest.ucInitPos = 2;
		pifLog_Printf(LT_INFO, "InitPos: Start");
		break;

	case 2:
		if (!s_stDutyMotorTest.ucStage) {
			if (s_switch[0].parent._curr_state == ON) {
				pifLog_Printf(LT_INFO, "InitPos: Find");
				s_stDutyMotorTest.ucInitPos = 0;
			}
			else {
				if (pifDutyMotorSpeed_Start(&g_motor, 0, unTime)) {
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
			if (pifDutyMotorSpeed_Start(&g_motor, 1, unTime)) {
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
		pifLog_Printf(LT_ERROR, "InitPos: Error");
		s_stDutyMotorTest.ucInitPos = 0;
		break;
	}
	return 0;
}

void appSetup()
{
	static PifUart s_uart_log;
	static PifLed s_led_l;
	PifTask* p_task;

	pif_Init(NULL);

    if (!pifTaskManager_Init(8)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;						// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;						// 1ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;											// 500ms

    if (!pifDutyMotorSpeed_Init(&g_motor, PIF_ID_AUTO, &g_timer_1ms, 255, 100)) return;			// 100ms
    pifDutyMotorSpeed_AddStages(&g_motor, DUTY_MOTOR_STAGE_COUNT, s_stDutyMotorStages);
    g_motor.parent.act_set_duty = actSetDuty;
    g_motor.parent.act_set_direction = actSetDirection;
    g_motor.parent.act_operate_break = actOperateBreak;
    g_motor.parent.evt_stable = _evtStable;
    g_motor.parent.evt_stop = _evtStop;
    g_motor.parent.evt_error = _evtError;

    for (int i = 0; i < SWITCH_COUNT; i++) {
		if (!pifSensorSwitch_Init(&s_switch[i], PIF_ID_SWITCH + i, 0, actPhotoInterruptAcquire, &g_motor)) return;
	    if (!pifSensorSwitch_AttachTaskAcquire(&s_switch[i], TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
    }

    p_task = pifTaskManager_Add(TM_PERIOD_MS, 10, _taskInitPos, NULL, TRUE);					// 10ms
    if (!p_task) return;
    p_task->name = "InitPos";

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
