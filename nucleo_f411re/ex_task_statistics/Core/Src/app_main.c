#include "app_port.h"


#define REALTIME_PERIOD_US		1000		// 1ms
#define CONTROL_PERIOD_US		10000		// 10ms
#define CONTROL_LONG_PERIOD_US	30000		// 30ms, returned once every CONTROL_LONG_EVERY runs
#define CONTROL_LONG_EVERY		50
#define EVENT_EVERY				4			// control runs per trigger of the event task
#define BACKUP_STEP_US			20000		// 20ms, one line of the backup a step
#define BACKUP_SIZE				120			// Backups kept in RAM, 2 minutes
#define DUMP_PERIOD_US			2000		// 2ms between the runs of the VCD dump
#define DUMP_LINES_PER_RUN		8			// Lines one run of the dump hands to the log at most

// Steps of a backup. The TK line of task n is printed on step STEP_TASK + n.
#define STEP_HEAD				0			// Takes the backup and prints the BK line
#define STEP_REALTIME			1
#define STEP_IDLE				2
#define STEP_TASK				3


// One backup: everything the scheduler measured over one second.
typedef struct {
	uint32_t sequence;
	uint32_t time_sec;
	uint8_t load;
	uint8_t task_count;
	PifTaskStatistics task[TASK_SIZE];
	PifTaskRealtimeStatistics realtime;
	BOOL idle_valid;
	PifTaskIdleStatistics idle;
} Backup;

// Sending of the backups as a VCD file. pifTaskManager_GetVcdLine() makes the lines, and the dump
// task hands them to the log, so a line the log had no room for is kept and sent again.
typedef struct {
	BOOL active;
	BOOL pending;					// text holds a line still to be sent
	uint16_t record_count;
	uint32_t first_sequence;
	PifTaskVcd vcd;
	char text[PIF_TASK_VCD_LINE_SIZE];
} Dump;


static PifTask *s_task_event;
static PifTask *s_task_backup;
static PifTask *s_task_dump;
static Backup s_backup[BACKUP_SIZE];
static Backup *s_current;				// Backup the steps are filling
static BOOL s_filling = FALSE;			// s_current is a slot of the ring not complete yet
static uint8_t s_step = STEP_HEAD;
static uint32_t s_second = 0;			// Second the last backup was taken in
static BOOL s_resync = FALSE;			// The next turn of the second starts a collection over
static uint32_t s_sequence = 0;			// Complete backups in the ring
static uint32_t s_skip_count = 0;		// Backups not kept because a dump was reading the ring
static Dump s_dump;

static int _cmdVcd(int argc, char *argv[]);
static int _cmdStart(int argc, char *argv[]);
static int _cmdStop(int argc, char *argv[]);

static const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "vcd", _cmdVcd, "Send the backups as a VCD file", NULL },
	{ "start", _cmdStart, "Start a new collection of the backups", NULL },
	{ "stop", _cmdStop, "Stop the collection of the backups", NULL },

	{ NULL, NULL, NULL, NULL }
};


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

// A run of a different length every time, a longer period once in a while, and a trigger of the
// event task every EVENT_EVERY runs.
static uint32_t _taskControl(PifTask *p_task)
{
	static uint32_t count = 0;

	(void)p_task;

	count++;
	_busy(100 + (count * 37) % 400);
	if (count % EVENT_EVERY == 0) pifTask_SetTrigger(s_task_event, 0);
	// Left out of the average delta time, which is about the default period only.
	return (count % CONTROL_LONG_EVERY == 0) ? CONTROL_LONG_PERIOD_US : 0;
}

static uint32_t _taskEvent(PifTask *p_task)
{
	(void)p_task;

	_busy(100);
	return 0;
}

// Takes the statistics of the task the step is on, the n-th one in the ring.
static void _backupTask(PifTask *p_task, void *p_arg)
{
	uint8_t *p_index = (uint8_t *)p_arg;
	PifTaskStatistics *p_stat;

	// Against the step, which keeps still while the ring is walked, not against task_count, which the
	// matching task moves on to the index of the next one.
	if ((*p_index)++ != s_step - STEP_TASK) return;

	p_stat = &s_current->task[s_current->task_count++];
	// The maxima start over, so that the next backup holds those of its own interval.
	pifTask_GetStatistics(p_task, p_stat, TRUE);

	pifLog_Printf(LT_NONE, "TK,%u,%s,%c,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", p_stat->id,
			p_task->name ? p_task->name : "---",
			p_stat->mode == TM_REALTIME ? 'R' : p_stat->mode == TM_PERIOD ? 'P' : 'E',
			(unsigned long)p_stat->max_execution_time, _average(p_stat->average_execution_time),
			_average(p_stat->average_delta_time), (unsigned long)p_stat->max_delay,
			(unsigned long)p_stat->max_trigger_delay, (unsigned long)p_stat->execution_rate,
			(unsigned long)p_stat->block_time);
}

static void _backupHead()
{
	static Backup s_skipped;

	// A dump reads the ring from its oldest backup, which this one would overwrite once the ring
	// is full. The statistics are still taken, so that the next backup holds its own interval.
	if (s_dump.active) {
		s_current = &s_skipped;
		s_skip_count++;
	}
	else {
		s_current = &s_backup[s_sequence % BACKUP_SIZE];
		s_filling = TRUE;
	}
	s_current->sequence = s_sequence;
	s_current->time_sec = pif_timer1sec;
	s_current->load = pif_performance._task_load;
	s_current->task_count = 0;
	// Only the figures start over. The margin the scheduler learned is kept.
	pifTaskManager_GetRealtimeStatistics(&s_current->realtime, TRUE);
	s_current->idle_valid = pifTaskManager_GetIdleStatistics(&s_current->idle);
	if (s_current->idle_valid) pifTaskManager_ResetIdleMaxExecutionTime();

	pifLog_Printf(LT_NONE, "BK,%lu,%lu,%u\n", (unsigned long)s_current->sequence,
			(unsigned long)s_current->time_sec, s_current->load);
}

static void _resetTask(PifTask *p_task, void *p_arg)
{
	(void)p_arg;

	pifTask_ResetStatistics(p_task);
}

static void _resetStatistics()
{
	PifTaskRealtimeStatistics realtime;

	pifTaskManager_AllTask(_resetTask, NULL);
	pifTaskManager_GetRealtimeStatistics(&realtime, TRUE);
	pifTaskManager_ResetIdleMaxExecutionTime();
}

// Takes a backup a second and prints it as CSV a line a step. Printing a dozen lines at once holds
// the CPU for longer than the realtime period on a slow board. A step of one line is short, and
// as it is measured like any other task, its block time keeps it out of the margin before the
// realtime release. Task n is still read once a second, n steps after the BK line.
static uint32_t _taskBackup(PifTask *p_task)
{
	uint8_t index;

	(void)p_task;

	if (s_step == STEP_HEAD) {
		// A backup starts when the second turns rather than after a count of steps, which the
		// waits for the slack would stretch past a second.
		if (pif_timer1sec == s_second) return 0;
		s_second = pif_timer1sec;
		if (s_resync) {
			// What was measured while the collection stood still is not part of the first backup.
			_resetStatistics();
			s_resync = FALSE;
			return 0;
		}
		_backupHead();
	}
	else if (s_step == STEP_REALTIME) {
		pifLog_Printf(LT_NONE, "RT,%lu,%lu,%lu,%lu\n", (unsigned long)s_current->realtime.max_delay,
				(unsigned long)s_current->realtime.miss_count, (unsigned long)s_current->realtime.guard,
				(unsigned long)s_current->realtime.lapse_count);
	}
	else if (s_step == STEP_IDLE) {
		if (s_current->idle_valid) {
			pifLog_Printf(LT_NONE, "ID,%lu,%lu,%lu\n", (unsigned long)s_current->idle.max_execution_time,
					_average(s_current->idle.average_execution_time),
					_average(s_current->idle.average_delta_time));
		}
	}
	else if (s_step - STEP_TASK < pifTaskManager_Count()) {
		index = 0;
		pifTaskManager_AllTask(_backupTask, &index);
		if (s_current->task_count >= pifTaskManager_Count()) {
			// A blank line closes the backup, so the blocks stand apart on a terminal.
			pifLog_Print(LT_NONE, "\n");
			if (s_filling) {
				s_filling = FALSE;
				s_sequence++;
			}
			s_step = STEP_HEAD;
			return 0;
		}
	}
	s_step++;
	return 0;
}

// Gives the VCD file a backup, from the oldest one kept. The record points into the ring, which
// keeps still while the dump is active.
static BOOL _getRecord(uint16_t index, PifTaskStatisticsRecord *p_record, void *p_client)
{
	const Backup *p_backup = &s_backup[(s_dump.first_sequence + index) % BACKUP_SIZE];

	(void)p_client;

	if (index >= s_dump.record_count) return FALSE;
	p_record->time = p_backup->time_sec;
	p_record->load = p_backup->load;
	p_record->task_count = p_backup->task_count;
	p_record->p_task = p_backup->task;
	p_record->p_realtime = &p_backup->realtime;
	p_record->p_idle = p_backup->idle_valid ? &p_backup->idle : NULL;
	return TRUE;
}

// Hands the dump to the log a few lines a run. A line the log has no room for is dropped and
// counted by pifLog_DropCount(), so it is kept and sent again once the UART has caught up.
static uint32_t _taskDump(PifTask *p_task)
{
	uint32_t drop_count;
	int lines;

	// Sending the dump is not part of the load being measured.
	pifTask_IgnoreStatistics(p_task);

	for (lines = 0; lines < DUMP_LINES_PER_RUN; lines++) {
		if (!s_dump.pending) {
			s_dump.pending = pifTaskManager_GetVcdLine(&s_dump.vcd, s_dump.text, sizeof(s_dump.text)) > 0;
		}
		if (!s_dump.pending) {
			// The last lines may still fill the log, so the report waits for room as they did.
			pifLog_Enable();
			drop_count = pifLog_DropCount();
			pifLog_Printf(LT_INFO, "VCD: %u backups sent, %lu skipped", (unsigned int)s_dump.record_count,
					(unsigned long)s_skip_count);
			if (pifLog_DropCount() == drop_count) {
				s_dump.active = FALSE;
				pifTask_Pause(p_task);
			}
			return 0;
		}
		drop_count = pifLog_DropCount();
		pifLog_Print(LT_VCD, s_dump.text);
		s_dump.pending = pifLog_DropCount() != drop_count;
		if (s_dump.pending) break;
	}
	return 0;
}

static int _cmdVcd(int argc, char *argv[])
{
	uint16_t room;

	(void)argc;
	(void)argv;

	if (s_dump.active) return PIF_LOG_CMD_USER_ERROR;
	if (!s_sequence) {
		pifLog_Print(LT_NONE, "  No backup yet\n");
		return PIF_LOG_CMD_NO_ERROR;
	}

	// The slot the steps are filling holds the oldest backup of a full ring, half overwritten.
	room = s_filling ? BACKUP_SIZE - 1 : BACKUP_SIZE;
	s_dump.record_count = (s_sequence < room) ? s_sequence : room;
	s_dump.first_sequence = s_sequence - s_dump.record_count;
	if (!pifTaskManager_StartVcd(&s_dump.vcd, s_dump.record_count, _getRecord, NULL, "ex_task_statistics", "1 s")) {
		return PIF_LOG_CMD_USER_ERROR;
	}
	// The prompt of the terminal stays on the line the dump starts on, so the file starts below it.
	pif_Printf(s_dump.text, sizeof(s_dump.text), "\n");
	s_dump.pending = TRUE;
	s_dump.active = TRUE;
	// Only the VCD lines go out until the dump is over, so nothing else lands in the file.
	pifLog_Disable();
	pifTask_Resume(s_task_dump);
	return PIF_LOG_CMD_NO_ERROR;
}

// Starts a new collection, so the VCD file holds only the backups taken since.
static int _cmdStart(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	// The dump reads the ring from first_sequence, which a new collection would move.
	if (s_dump.active) return PIF_LOG_CMD_USER_ERROR;

	s_sequence = 0;
	s_skip_count = 0;
	s_filling = FALSE;
	s_step = STEP_HEAD;
	// The statistics start over when the second next turns, so the first backup, taken at the turn
	// after that, holds a full second.
	s_second = pif_timer1sec;
	s_resync = TRUE;
	pifTask_Resume(s_task_backup);
	return PIF_LOG_CMD_NO_ERROR;
}

// The backups taken so far are kept for the VCD file. One the steps were filling is left out of it.
static int _cmdStop(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pifTask_Pause(s_task_backup);
	return PIF_LOG_CMD_NO_ERROR;
}

BOOL appSetup()
{
	PifTask *p_task;

	if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) return FALSE;		// 32bytes

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_REALTIME, REALTIME_PERIOD_US, _taskRealtime, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "realtime";

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, CONTROL_PERIOD_US, _taskControl, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "control";

	s_task_event = pifTaskManager_Add(PIF_ID_AUTO, TM_EXTERNAL, 0, _taskEvent, NULL, FALSE);
	if (!s_task_event) return FALSE;
	s_task_event->name = "event";

	// The collection starts at boot, and the start and stop commands set when it runs after that.
	s_task_backup = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, BACKUP_STEP_US, _taskBackup, NULL, TRUE);
	if (!s_task_backup) return FALSE;
	s_task_backup->name = "backup";

	s_task_dump = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, DUMP_PERIOD_US, _taskDump, NULL, FALSE);
	if (!s_task_dump) return FALSE;
	s_task_dump->name = "dump";

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***        ex_task_statistics        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d\n", pifTaskManager_Count(), TASK_SIZE);
	pifLog_Print(LT_NONE, "BK,sequence,time(s),load(%)\n");
	pifLog_Print(LT_NONE, "RT,max delay,miss,guard,lapse\n");
	pifLog_Print(LT_NONE, "ID,max,average,delta\n");
	pifLog_Print(LT_NONE, "TK,id,name,mode,PrcM,PrcA,Delta,Dly,TrgM,Rate,Blk\n\n");
	return TRUE;
}
