#include "app_port.h"

#include <stdlib.h>
#include <string.h>


#define DEMO_TIME				1500	// 1500ms a pose


PifLed g_led_l;
PifServo g_servo;
PifTimerManager g_timer_1ms;

// Poses the demo goes through, one angle a servo in 0.1 degrees.
static const int16_t c_demo_pose[][SERVO_COUNT] = {
	{    0,  900, 1800 },
	{ 1800,  900,    0 },
	{  900,    0,  900 },
	{  900, 1800,  900 }
};

#define DEMO_POSE_COUNT			(sizeof(c_demo_pose) / sizeof(c_demo_pose[0]))

static BOOL s_demo = FALSE;
static uint8_t s_demo_pose;


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

// Reads degrees with up to one decimal, as "90", "-45" or "12.5", into 0.1 degrees.
static BOOL _parseAngle(const char *p_text, int16_t *p_angle)
{
	char *p_end;
	long value;

	value = strtol(p_text, &p_end, 10);
	if (p_end == p_text || value < -3276 / 10 || value > 3276 / 10) return FALSE;
	value *= 10;
	if (*p_end == '.') {
		if (p_end[1] < '0' || p_end[1] > '9') return FALSE;
		value += *p_text == '-' ? -(p_end[1] - '0') : p_end[1] - '0';
		p_end += 2;
	}
	if (*p_end) return FALSE;
	*p_angle = value;
	return TRUE;
}

static BOOL _parseNumber(const char *p_text, long min, long max, long *p_value)
{
	char *p_end;

	*p_value = strtol(p_text, &p_end, 10);
	return p_end != p_text && !*p_end && *p_value >= min && *p_value <= max;
}

static BOOL _parseIndex(const char *p_text, uint8_t *p_index)
{
	long value;

	if (!_parseNumber(p_text, 0, SERVO_COUNT - 1, &value)) return FALSE;
	*p_index = value;
	return TRUE;
}

static BOOL _parseOnOff(const char *p_text, BOOL *p_on)
{
	if (!strcmp(p_text, "on")) *p_on = TRUE;
	else if (!strcmp(p_text, "off")) *p_on = FALSE;
	else return FALSE;
	return TRUE;
}

static void _printStatus(uint8_t index)
{
	const PifServoChannel *p_channel = pifServo_GetChannel(&g_servo, index);
	char angle[8], min_angle[8], max_angle[8];

	if (!pifServo_IsAttached(&g_servo, index)) {
		pifLog_Printf(LT_NONE, "%u: detached\n", index);
		return;
	}
	_formatAngle(angle, pifServo_GetAngle(&g_servo, index));
	_formatAngle(min_angle, p_channel->_min_angle);
	_formatAngle(max_angle, p_channel->_max_angle);
	pifLog_Printf(LT_NONE, "%u: %s deg %u us%s, range %s..%s deg = %u..%u us, trim %d%s%s\n", index, angle,
			p_channel->_pulse, pifServo_IsMoving(&g_servo, index) ? " moving" : "", min_angle, max_angle,
			p_channel->_min_pulse, p_channel->_max_pulse, p_channel->_trim, p_channel->_reverse ? ", reverse" : "",
			p_channel->_profile == SVP_EASE ? ", ease" : "");
}

static void _nextDemoPose()
{
	pifServo_MoveSync(&g_servo, c_demo_pose[s_demo_pose], DEMO_TIME);
	s_demo_pose = (s_demo_pose + 1) % DEMO_POSE_COUNT;
}

static void _evtServoArrive(PifServo *p_owner, uint8_t index)
{
	char angle[8];
	uint8_t i;

	if (s_demo) {
		// The servos of a pose arrive together, and this is the last of them once none is moving.
		for (i = 0; i < SERVO_COUNT; i++) {
			if (pifServo_IsMoving(p_owner, i)) return;
		}
		_nextDemoPose();
		return;
	}

	_formatAngle(angle, pifServo_GetAngle(p_owner, index));
	pifLog_Printf(LT_INFO, "Servo %u arrived at %s deg", index, angle);
}

// The commands that set or move a servo end the demo, which would move it away again.
static void _stopDemo()
{
	s_demo = FALSE;
}

static int _cmdAngle(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	int16_t angle;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseAngle(argv[1], &angle)) return PIF_LOG_CMD_INVALID_ARG;

	_stopDemo();
	pifServo_SetAngle(&g_servo, index, angle);
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdMove(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	int16_t angle;
	long time;

	(void)p_entry;

	if (argc < 3) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseAngle(argv[1], &angle) || !_parseNumber(argv[2], 0, 65535, &time)) {
		return PIF_LOG_CMD_INVALID_ARG;
	}

	_stopDemo();
	pifServo_MoveTo(&g_servo, index, angle, time);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdSpeed(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	int16_t angle;
	long speed;

	(void)p_entry;

	if (argc < 3) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseAngle(argv[1], &angle) || !_parseNumber(argv[2], 1, 65535, &speed)) {
		return PIF_LOG_CMD_INVALID_ARG;
	}

	_stopDemo();
	if (!pifServo_MoveAtSpeed(&g_servo, index, angle, speed)) return PIF_LOG_CMD_INVALID_ARG;
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdSync(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	int16_t angles[SERVO_COUNT];
	long time;
	uint8_t i;

	(void)p_entry;

	if (argc < 1 + SERVO_COUNT) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseNumber(argv[0], 0, 65535, &time)) return PIF_LOG_CMD_INVALID_ARG;
	for (i = 0; i < SERVO_COUNT; i++) {
		if (!strcmp(argv[1 + i], "-")) angles[i] = PIF_SERVO_KEEP;
		else if (!_parseAngle(argv[1 + i], &angles[i])) return PIF_LOG_CMD_INVALID_ARG;
	}

	_stopDemo();
	pifServo_MoveSync(&g_servo, angles, time);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdPulse(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	long pulse;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseNumber(argv[1], SERVO_PULSE_MIN, SERVO_PULSE_MAX, &pulse)) {
		return PIF_LOG_CMD_INVALID_ARG;
	}

	_stopDemo();
	pifServo_SetPulse(&g_servo, index, pulse);
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdStop(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;

	(void)p_entry;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index)) return PIF_LOG_CMD_INVALID_ARG;

	_stopDemo();
	pifServo_Stop(&g_servo, index);
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdDetach(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;

	(void)p_entry;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;

	_stopDemo();
	if (!strcmp(argv[0], "all")) {
		for (index = 0; index < SERVO_COUNT; index++) pifServo_Detach(&g_servo, index);
		return PIF_LOG_CMD_NO_ERROR;
	}
	if (!_parseIndex(argv[0], &index)) return PIF_LOG_CMD_INVALID_ARG;
	pifServo_Detach(&g_servo, index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdTrim(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	long trim;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseNumber(argv[1], -300, 300, &trim)) return PIF_LOG_CMD_INVALID_ARG;

	if (!pifServo_SetTrim(&g_servo, index, trim)) return PIF_LOG_CMD_INVALID_ARG;
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdRange(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	const PifServoChannel *p_channel;
	uint8_t index;
	long min_pulse, max_pulse;
	int16_t min_angle, max_angle;

	(void)p_entry;

	if (argc < 3) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseNumber(argv[1], SERVO_PULSE_MIN, SERVO_PULSE_MAX, &min_pulse) ||
			!_parseNumber(argv[2], SERVO_PULSE_MIN, SERVO_PULSE_MAX, &max_pulse)) return PIF_LOG_CMD_INVALID_ARG;
	p_channel = pifServo_GetChannel(&g_servo, index);
	min_angle = p_channel->_min_angle;
	max_angle = p_channel->_max_angle;
	if (argc >= 5 && (!_parseAngle(argv[3], &min_angle) || !_parseAngle(argv[4], &max_angle))) {
		return PIF_LOG_CMD_INVALID_ARG;
	}

	if (!pifServo_SetRange(&g_servo, index, min_pulse, max_pulse, min_angle, max_angle)) return PIF_LOG_CMD_INVALID_ARG;
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdReverse(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	BOOL on;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseIndex(argv[0], &index) || !_parseOnOff(argv[1], &on)) return PIF_LOG_CMD_INVALID_ARG;

	pifServo_SetReverse(&g_servo, index, on);
	_printStatus(index);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdEase(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t index;
	BOOL on;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseOnOff(argv[1], &on)) return PIF_LOG_CMD_INVALID_ARG;

	if (!strcmp(argv[0], "all")) {
		for (index = 0; index < SERVO_COUNT; index++) pifServo_SetProfile(&g_servo, index, on ? SVP_EASE : SVP_LINEAR);
		return PIF_LOG_CMD_NO_ERROR;
	}
	if (!_parseIndex(argv[0], &index)) return PIF_LOG_CMD_INVALID_ARG;
	pifServo_SetProfile(&g_servo, index, on ? SVP_EASE : SVP_LINEAR);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdDemo(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	BOOL on;

	(void)p_entry;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;
	if (!_parseOnOff(argv[0], &on)) return PIF_LOG_CMD_INVALID_ARG;

	if (on && !s_demo) {
		s_demo = TRUE;
		s_demo_pose = 0;
		_nextDemoPose();
	}
	else if (!on && s_demo) {
		// The servos finish the pose under way and stay there.
		s_demo = FALSE;
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdStatus(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	uint8_t i;

	(void)p_entry;

	(void)argc;
	(void)argv;

	for (i = 0; i < SERVO_COUNT; i++) _printStatus(i);
	return PIF_LOG_CMD_NO_ERROR;
}

static const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", _cmdStatus, "Print the servos", NULL },
	{ "angle", _cmdAngle, "Set an angle at once", "<ch> <deg>" },
	{ "move", _cmdMove, "Move to an angle in a time", "<ch> <deg> <ms>" },
	{ "speed", _cmdSpeed, "Move to an angle at a speed", "<ch> <deg> <deg/s>" },
	{ "sync", _cmdSync, "Move all together, - keeps one", "<ms> <deg|-> <deg|-> <deg|->" },
	{ "pulse", _cmdPulse, "Set a pulse at once", "<ch> <us>" },
	{ "stop", _cmdStop, "Stop a move where it is", "<ch>" },
	{ "detach", _cmdDetach, "Stop the pulse", "<ch> | all" },
	{ "trim", _cmdTrim, "Offset the pulses", "<ch> <us>" },
	{ "range", _cmdRange, "Set the range", "<ch> <min us> <max us> [<min deg> <max deg>]" },
	{ "reverse", _cmdReverse, "Swap the ends", "<ch> on | off" },
	{ "ease", _cmdEase, "Ease in and out", "<ch> | all on | off" },
	{ "demo", _cmdDemo, "Go through poses", "on | off" },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) return FALSE;							// 32bytes

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    g_servo.evt_arrive = _evtServoArrive;
    g_servo._p_task->name = "Servo";

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***             ex_servo             ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");

	// No pulse goes out before a command, so the servos keep still at power up.
	pifLog_Print(LT_INFO, "Servos on pins " SERVO_PINS ", detached. Try: move 0 90 1000, or demo on");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
