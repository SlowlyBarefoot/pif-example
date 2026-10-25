#include "app_port.h"


#define REALTIME_PERIOD_US		2000		// 2ms, as a CSV line takes about 1.2ms to format on the 16MHz AVR
#define CONTROL_PERIOD_US		10000		// 10ms
#define CONTROL_LONG_PERIOD_US	30000		// 30ms, returned once every CONTROL_LONG_EVERY runs
#define CONTROL_LONG_EVERY		50
#define BACKUP_STEP_US			20000		// 20ms, one line of the backup a step

// Steps of a backup. The TK line of task n is printed on step STEP_TASK + n.
#define STEP_HEAD				0			// Takes the backup and prints the BK line
#define STEP_REALTIME			1
#define STEP_TASK				2


// The 2KB of the Nano holds neither the backups nor the command line, so each backup is printed as
// CSV as it is taken, from boot on. The VCD file, the start and stop commands and the event task of
// the larger boards are left out. The trigger delay is still measured on the UART task of the log.
// No idle callback is set, so there is no ID line.

static PifTaskRealtimeStatistics s_realtime;
static uint8_t s_step = STEP_HEAD;
static uint32_t s_second = 0;			// Second the last backup was taken in
static uint32_t s_sequence = 0;


// Holds the CPU for a while, as real work would.
static void _busy(uint32_t us)
{
	uint32_t start = (*pif_act_timer1us)();

	while ((*pif_act_timer1us)() - start < us);
}

// An average that is not measured yet is printed as 0, as pifTaskManager_Print() does.
static unsigned long _average(uint32_t value)
{
	return (value == PIF_TASK_AVERAGE_NONE) ? 0UL : value;
}

static uint32_t _taskRealtime(PifTask *p_task)
{
	(void)p_task;

	actRealtimeSignal(ON);
	_busy(50);
	actRealtimeSignal(OFF);
	return 0;
}

// A run of a different length every time, and a longer period once in a while.
static uint32_t _taskControl(PifTask *p_task)
{
	static uint32_t count = 0;

	(void)p_task;

	count++;
	_busy(100 + (count * 37) % 400);
	// Left out of the average delta time, which is about the default period only.
	return (count % CONTROL_LONG_EVERY == 0) ? CONTROL_LONG_PERIOD_US : 0;
}

// Prints the task the step is on, the n-th one in the ring.
static void _backupTask(PifTask *p_task, void *p_arg)
{
	uint8_t *p_index = (uint8_t *)p_arg;
	PifTaskStatistics stat;

	if ((*p_index)++ != s_step - STEP_TASK) return;

	// The maxima start over, so that the next backup holds those of its own interval.
	pifTask_GetStatistics(p_task, &stat, TRUE);

	pifLog_Printf(LT_NONE, "TK,%u,%s,%c,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", stat.id,
			p_task->name ? p_task->name : "---",
			stat.mode == TM_REALTIME ? 'R' : stat.mode == TM_PERIOD ? 'P' : 'E',
			(unsigned long)stat.max_execution_time, _average(stat.average_execution_time),
			_average(stat.average_delta_time), (unsigned long)stat.max_delay,
			(unsigned long)stat.max_trigger_delay, (unsigned long)stat.execution_rate,
			(unsigned long)stat.block_time);
}

// Takes a backup a second and prints it as CSV a line a step, so the log needs room for one line
// only. Printing them all at once holds the CPU for longer than the realtime period on the AVR. A
// step of one line is short, and as it is measured like any other task, its block time keeps it out
// of the margin before the realtime release. Task n is still read once a second, n steps after the
// BK line.
static uint32_t _taskBackup(PifTask *p_task)
{
	uint8_t index;

	(void)p_task;

	if (s_step == STEP_HEAD) {
		// A backup starts when the second turns rather than after a count of steps, which the
		// waits for the slack would stretch past a second.
		if (pif_timer1sec == s_second) return 0;
		s_second = pif_timer1sec;
		// Only the figures start over. The margin the scheduler learned is kept.
		pifTaskManager_GetRealtimeStatistics(&s_realtime, TRUE);
		pifLog_Printf(LT_NONE, "BK,%lu,%lu,%u\n", (unsigned long)s_sequence++, (unsigned long)pif_timer1sec,
				pif_performance._task_load);
	}
	else if (s_step == STEP_REALTIME) {
		pifLog_Printf(LT_NONE, "RT,%lu,%lu,%lu,%lu\n", (unsigned long)s_realtime.max_delay,
				(unsigned long)s_realtime.miss_count, (unsigned long)s_realtime.guard,
				(unsigned long)s_realtime.lapse_count);
	}
	else if (s_step - STEP_TASK < pifTaskManager_Count()) {
		index = 0;
		pifTaskManager_AllTask(_backupTask, &index);
		if (s_step - STEP_TASK == pifTaskManager_Count() - 1) {
			// A blank line closes the backup, so the blocks stand apart on a terminal.
			pifLog_Print(LT_NONE, "\n");
			s_step = STEP_HEAD;
			return 0;
		}
	}
	s_step++;
	return 0;
}

BOOL appSetup()
{
	PifTask *p_task;

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_REALTIME, REALTIME_PERIOD_US, _taskRealtime, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "realtime";

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, CONTROL_PERIOD_US, _taskControl, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "control";

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, BACKUP_STEP_US, _taskBackup, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "backup";

	// Every string takes RAM on the AVR, so the banner of the other boards is cut to one line.
	pifLog_Print(LT_NONE, "\n\nex_task_statistics\n");
	pifLog_Printf(LT_NONE, "%s %s\n", __DATE__, __TIME__);
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
	pifLog_Print(LT_NONE, "BK,sequence,time(s),load(%)\n");
	pifLog_Print(LT_NONE, "RT,max delay,miss,guard,lapse\n");
	pifLog_Print(LT_NONE, "TK,id,name,mode,PrcM,PrcA,Delta,Dly,TrgM,Rate,Blk\n\n");
	return TRUE;
}
