#include "app_port.h"

#include <stdlib.h>
#include <string.h>


// The X of the stick is the angle of the servo, and the Y is how fast it goes there: slow at the bottom,
// SPEED_MID at rest and fast at the top.
#define SPEED_MIN				20		// deg/s
#define SPEED_MID				90		// deg/s
#define SPEED_MAX				360		// deg/s

#define DEAD_ZONE				50		// of PIF_JOYSTICK_RANGE


PifLed g_led_l;
PifServo g_servo;
PifJoystick g_joystick;
PifTimerManager g_timer_1ms;

static PifTask *s_p_task_monitor;
static int16_t s_target;				// 0.1 degrees
static uint16_t s_speed;				// deg/s


// Writes 0.1 degrees as "-12.5" with a NUL.
static void _formatAngle(char *p_buffer, int16_t angle)
{
	uint16_t value;
	int offset = 0;

	if (angle < 0) {
		p_buffer[offset++] = '-';
		value = -(int32_t)angle;
	}
	else {
		value = angle;
	}
	offset += pif_DecToString(p_buffer + offset, value / 10, 0);
	p_buffer[offset++] = '.';
	p_buffer[offset++] = '0' + value % 10;
	p_buffer[offset] = 0;
}

// Writes the direction as two letters, "UR" or "L " for instance, or "--" in the middle.
static void _formatDirection(char *p_buffer, uint8_t direction)
{
	p_buffer[0] = direction & JD_UP ? 'U' : direction & JD_DOWN ? 'D' : ' ';
	p_buffer[1] = direction & JD_LEFT ? 'L' : direction & JD_RIGHT ? 'R' : ' ';
	if (direction == JD_NONE) p_buffer[0] = p_buffer[1] = '-';
	p_buffer[2] = 0;
}

static BOOL _parseNumber(const char *p_text, long min, long max, long *p_value)
{
	char *p_end;

	*p_value = strtol(p_text, &p_end, 10);
	return p_end != p_text && !*p_end && *p_value >= min && *p_value <= max;
}

static BOOL _parseOnOff(const char *p_text, BOOL *p_on)
{
	if (!strcmp(p_text, "on")) *p_on = TRUE;
	else if (!strcmp(p_text, "off")) *p_on = FALSE;
	else return FALSE;
	return TRUE;
}

// What the servo is told, and where it is now, so the stick can be followed without a servo.
static void _printStatus()
{
	const PifServoChannel *p_channel = pifServo_GetChannel(&g_servo, 0);
	char target[8], angle[8], direction[3];

	_formatAngle(target, s_target);
	_formatAngle(angle, pifServo_GetAngle(&g_servo, 0));
	_formatDirection(direction, g_joystick._direction);
	pifLog_Printf(LT_NONE, "raw %4d %4d pos %5d %5d %s tgt %5s cur %5s deg %3u dps %4u us\n",
			g_joystick._raw[JA_X], g_joystick._raw[JA_Y], g_joystick._position[JA_X], g_joystick._position[JA_Y],
			direction, target, angle, s_speed, p_channel->_pulse);
}

static uint32_t _doTaskMonitor(PifTask *p_task)
{
	(void)p_task;

	_printStatus();
	return 0;
}

static void _evtJoystickMove(PifJoystick *p_owner, int16_t x, int16_t y)
{
	const PifServoChannel *p_channel = pifServo_GetChannel(&g_servo, 0);
	int32_t span = p_channel->_max_angle - p_channel->_min_angle;

	(void)p_owner;

	// -PIF_JOYSTICK_RANGE..+PIF_JOYSTICK_RANGE onto the range of the servo, its middle at rest.
	s_target = p_channel->_min_angle + span / 2 + (int32_t)x * (span / 2) / PIF_JOYSTICK_RANGE;
	if (y >= 0) s_speed = SPEED_MID + (int32_t)y * (SPEED_MAX - SPEED_MID) / PIF_JOYSTICK_RANGE;
	else s_speed = SPEED_MID + (int32_t)y * (SPEED_MID - SPEED_MIN) / PIF_JOYSTICK_RANGE;

	// A move under way is replaced from where it got to, so the servo follows the stick at the new speed.
	pifServo_MoveAtSpeed(&g_servo, 0, s_target, s_speed);
}

static void _evtJoystickDirection(PifJoystick *p_owner, uint8_t direction)
{
	char text[3];

	(void)p_owner;

	_formatDirection(text, direction);
	pifLog_Printf(LT_INFO, "Direction %s", text);
}

static int _cmdStatus(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	_printStatus();
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdMonitor(int argc, char *argv[])
{
	BOOL on;
	long period;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseOnOff(argv[0], &on)) return PIF_LOG_CMD_INVALID_ARG;
	if (argc >= 2) {
		if (!_parseNumber(argv[1], 20, 10000, &period)) return PIF_LOG_CMD_INVALID_ARG;
		pifTask_ChangePeriod(s_p_task_monitor, period * 1000UL);
	}

	if (on) pifTask_Resume(s_p_task_monitor);
	else pifTask_Pause(s_p_task_monitor);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdCenter(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (!pifJoystick_CalibrateCenter(&g_joystick)) {
		pifLog_Print(LT_ERROR, "Center not taken: leave the stick at rest");
		return PIF_LOG_CMD_NO_ERROR;
	}
	pifLog_Printf(LT_INFO, "Center %d %d", g_joystick._cal[JA_X]._center, g_joystick._cal[JA_Y]._center);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdInvert(int argc, char *argv[])
{
	PifJoystickAxis axis;
	BOOL on;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!strcmp(argv[0], "x")) axis = JA_X;
	else if (!strcmp(argv[0], "y")) axis = JA_Y;
	else return PIF_LOG_CMD_INVALID_ARG;
	if (!_parseOnOff(argv[1], &on)) return PIF_LOG_CMD_INVALID_ARG;

	pifJoystick_SetInvert(&g_joystick, axis, on);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdDeadZone(int argc, char *argv[])
{
	long value;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseNumber(argv[0], 0, PIF_JOYSTICK_RANGE - 1, &value)) return PIF_LOG_CMD_INVALID_ARG;

	pifJoystick_SetDeadZone(&g_joystick, value);
	return PIF_LOG_CMD_NO_ERROR;
}

static const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", _cmdStatus, "Print the stick and the servo", NULL },
	{ "monitor", _cmdMonitor, "Print them periodically", "on | off [<ms>]" },
	{ "center", _cmdCenter, "Take the stick at rest as center", NULL },
	{ "invert", _cmdInvert, "Turn an axis round", "x | y on | off" },
	{ "deadzone", _cmdDeadZone, "Set the dead zone", "<0..999>" },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) return FALSE;							// 32bytes

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    g_servo._p_task->name = "Servo";

    // The stick is at rest at power up, so that reading is its center.
    if (!pifJoystick_CalibrateCenter(&g_joystick)) return FALSE;
    if (!pifJoystick_SetDeadZone(&g_joystick, DEAD_ZONE)) return FALSE;
    g_joystick.evt_move = _evtJoystickMove;
    g_joystick.evt_direction = _evtJoystickDirection;
    if (!pifJoystick_Start(&g_joystick, NULL)) return FALSE;

    s_p_task_monitor = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, MONITOR_PERIOD * 1000UL, _doTaskMonitor, NULL, TRUE);
    if (!s_p_task_monitor) return FALSE;
    s_p_task_monitor->name = "Monitor";

    // The servo goes to the middle of its range, where the stick at rest points it.
    s_target = (pifServo_GetChannel(&g_servo, 0)->_min_angle + pifServo_GetChannel(&g_servo, 0)->_max_angle) / 2;
    s_speed = SPEED_MID;
    pifServo_SetAngle(&g_servo, 0, s_target);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         ex_joystick_servo        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");

	pifLog_Print(LT_INFO, "Joystick " JOYSTICK_PINS ", servo " SERVO_PINS);
	pifLog_Print(LT_INFO, "X = angle, Y = speed. monitor off stops the lines");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
