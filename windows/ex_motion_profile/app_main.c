#include "app_port.h"

#include <math.h>


#define V_MAX					2000.0f		// steps/s
#define A_MAX					4000.0f		// steps/s^2
#define D_MAX					8000.0f		// steps/s^2

#define LOG_PERIOD				100			// ms
#define PAUSE					1000		// ms between the moves


typedef enum EnStepAction
{
	SA_MOVE,			// Moves to the target
	SA_RETARGET,		// Moves to the target, after the given time of the move before
	SA_STOP				// Stops, after the given time of the move before
} StepAction;

typedef struct StStepCommand
{
	StepAction action;
	float target;		// steps
	float retarget;		// steps, for SA_RETARGET
	uint32_t after;		// ms, for SA_RETARGET and SA_STOP
	const char *p_name;
} StepCommand;


PifTimerManager g_timer_1ms;

static PifMotionProfile s_profile;

static const StepCommand c_commands[] = {
	{ SA_MOVE, 3200.0f, 0.0f, 0, "trapezoid to 3200" },
	{ SA_MOVE, 3400.0f, 0.0f, 0, "triangle to 3400 (too short for V_MAX)" },
	{ SA_MOVE, 0.0f, 0.0f, 0, "back to 0" },
	{ SA_RETARGET, 4000.0f, 0.0f, 900, "to 4000, turned back to 0 at 900ms" },
	{ SA_RETARGET, 2000.0f, 6000.0f, 600, "to 2000, carried on to 6000 at 600ms" },
	{ SA_STOP, 0.0f, 0.0f, 800, "to 0, stopped at 800ms" },
	{ SA_MOVE, 0.0f, 0.0f, 0, "home to 0" },
};

static uint8_t s_command;
static int8_t s_phase;			// 0: first move of the command, 1: after the retarget or the stop, -1: pause
static uint32_t s_start;		// pif_cumulative_timer1ms of the start of the move
static int32_t s_step;			// Steps put out
static uint32_t s_pulses;		// Pulses of the command
static uint32_t s_logged;


// Starts the move to target from where the last sample left the profile.
static void _move(float target)
{
	float first;
	int8_t dir;

	pifMotionProfile_Move(&s_profile, target);
	s_start = pif_cumulative_timer1ms;

	// The time of each step, as a board would load into the compare register of a timer. Only the first one is
	// shown: at rest, it is sqrt(2 / A_MAX) s. A move that runs goes on the way it runs, whatever the target.
	if (s_profile._velocity != 0.0f) dir = s_profile._velocity > 0.0f ? 1 : -1;
	else dir = target > s_step ? 1 : -1;
	first = pifMotionProfile_TimeAtPosition(&s_profile, s_step + dir);
	pifLog_Printf(LT_INFO, "Move %ld -> %ld: %lu ms, first step at %lu us", (long)s_step, (long)target,
			(unsigned long)(pifMotionProfile_GetDuration(&s_profile) * 1000.0f + 0.5f),
			(unsigned long)(first * 1e6f + 0.5f));
}

static void _startCommand()
{
	const StepCommand *p_command = &c_commands[s_command];

	pifLog_Printf(LT_INFO, "[%u] %s", s_command + 1, p_command->p_name);
	s_pulses = 0;
	s_phase = 0;
	_move(p_command->target);
}

static uint32_t _taskStepMotor(PifTask *p_task)
{
	const StepCommand *p_command = &c_commands[s_command];
	uint32_t elapsed = pif_cumulative_timer1ms - s_start;
	int32_t position;

	(void)p_task;

	if (s_phase < 0) {
		if (elapsed >= PAUSE) {
			s_command = (s_command + 1) % (sizeof(c_commands) / sizeof(c_commands[0]));
			_startCommand();
		}
		return 0;
	}

	pifMotionProfile_Sample(&s_profile, elapsed / 1000.0f);

	// The pulses up to the sampled position. On a board a timer puts them out at their times instead, so that
	// they are even within the millisecond.
	position = (int32_t)lroundf(s_profile._position);
	while (s_step != position) {
		int8_t dir = position > s_step ? 1 : -1;

		portStepPulse(dir);
		s_step += dir;
		s_pulses++;
	}

	if (pif_cumulative_timer1ms - s_logged >= LOG_PERIOD) {
		s_logged = pif_cumulative_timer1ms;
		pifLog_Printf(LT_NONE, "\n  %4lu ms  step %6ld  speed %5ld steps/s  accel %6ld", (unsigned long)elapsed,
				(long)s_step, (long)lroundf(s_profile._velocity), (long)lroundf(s_profile._acceleration));
	}

	if (s_phase == 0 && p_command->action != SA_MOVE && elapsed >= p_command->after) {
		// The sample above holds the state to plan from.
		s_phase = 1;
		if (p_command->action == SA_STOP) {
			pifMotionProfile_Stop(&s_profile);
			s_start = pif_cumulative_timer1ms;
			pifLog_Printf(LT_INFO, "Stop in %lu ms", (unsigned long)(pifMotionProfile_GetDuration(&s_profile) * 1000.0f + 0.5f));
		}
		else _move(p_command->retarget);
		return 0;
	}

	if (s_profile._done) {
		pifLog_Printf(LT_INFO, "Done: step %ld, %lu pulses", (long)s_step, (unsigned long)s_pulses);
		s_phase = -1;
		s_start = pif_cumulative_timer1ms;
	}
	return 0;
}

BOOL appSetup()
{
	if (!pifMotionProfile_Init(&s_profile, V_MAX, A_MAX, D_MAX)) return FALSE;
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskStepMotor, NULL, TRUE)) return FALSE;	// 1ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***        ex_motion_profile         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "V_MAX=%u A_MAX=%u D_MAX=%u", (unsigned)V_MAX, (unsigned)A_MAX, (unsigned)D_MAX);

	// The first command starts from the task after a pause, once the banner has left the log buffer.
	s_command = sizeof(c_commands) / sizeof(c_commands[0]) - 1;
	s_phase = -1;
	return TRUE;
}
