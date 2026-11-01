#include "app_port.h"

#include <stdlib.h>
#include <string.h>


#define RAM_BOOT_COUNT			0		// Address in the RAM of the DS1302

#define SYNC_PERIOD				60		// 60sec
#define SHOW_PERIOD				1000	// 1000ms


PifLed g_led_l;
PifDs1302 g_ds1302;
PifTimerManager g_timer_1ms;

static PifTask *s_p_task_show;


// Writes "YYYY-MM-DD hh:mm:ss" with a NUL.
static void _format(char *p_buffer, const PifDateTime *p_datetime)
{
	int offset = 0;

	offset += pif_DecToString(p_buffer + offset, 2000 + p_datetime->year, 4);
	p_buffer[offset++] = '-';
	offset += pif_DecToString(p_buffer + offset, p_datetime->month, 2);
	p_buffer[offset++] = '-';
	offset += pif_DecToString(p_buffer + offset, p_datetime->day, 2);
	p_buffer[offset++] = ' ';
	offset += pif_DecToString(p_buffer + offset, p_datetime->hour, 2);
	p_buffer[offset++] = ':';
	offset += pif_DecToString(p_buffer + offset, p_datetime->minute, 2);
	p_buffer[offset++] = ':';
	offset += pif_DecToString(p_buffer + offset, p_datetime->second, 2);
	p_buffer[offset] = 0;
}

// Takes the date and time of the build from __DATE__ ("Nov  1 2026") and __TIME__ ("21:30:05").
static void _getBuildTime(PifDateTime *p_datetime)
{
	const char *p_date = __DATE__;
	const char *p_time = __TIME__;
	int i;

	p_datetime->month = 1;
	for (i = 0; i < 12; i++) {
		if (!strncmp(p_date, kPifMonth3[i], 3)) {
			p_datetime->month = i + 1;
			break;
		}
	}
	p_datetime->day = (p_date[4] == ' ' ? 0 : (p_date[4] - '0') * 10) + p_date[5] - '0';
	p_datetime->year = (p_date[9] - '0') * 10 + p_date[10] - '0';
	p_datetime->hour = (p_time[0] - '0') * 10 + p_time[1] - '0';
	p_datetime->minute = (p_time[3] - '0') * 10 + p_time[4] - '0';
	p_datetime->second = (p_time[6] - '0') * 10 + p_time[7] - '0';
	p_datetime->millisecond = 0;
}

// Shows the time of the chip next to pif_datetime, which the sync task keeps in step with it.
static void _show()
{
	PifDateTime rtc, sys;
	char rtc_text[20], sys_text[24];
	int offset;

	if (!pifRtc_GetDateTime(&g_ds1302.parent, &rtc)) {
		pifLog_Printf(LT_ERROR, "RTC read failed: E=%d", pif_error);
		return;
	}
	sys = *(PifDateTime *)&pif_datetime;
	_format(rtc_text, &rtc);
	_format(sys_text, &sys);
	offset = strlen(sys_text);
	sys_text[offset++] = '.';
	offset += pif_DecToString(sys_text + offset, pif_timer1ms, 3);
	sys_text[offset] = 0;
	pifLog_Printf(LT_INFO, "RTC=%s SYS=%s", rtc_text, sys_text);
}

static uint32_t _taskShow(PifTask *p_task)
{
	(void)p_task;

	_show();
	return 0;
}

// Logs each correction of pif_datetime by the sync task. 1 is the usual alignment to the tick of the chip.
static void _evtRtcSync(PifRtc *p_owner, int32_t diff, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_issuer;

	pifLog_Printf(LT_INFO, "RTC sync: %ld sec", (long)diff);
}

// Reads count numbers separated by sep, as "2026-11-01" or "21:30".
static BOOL _parseFields(const char *p_text, char sep, uint16_t *p_value, int count)
{
	char *p_end;
	int i;

	for (i = 0; i < count; i++) {
		p_value[i] = strtoul(p_text, &p_end, 10);
		if (p_end == p_text) return FALSE;
		if (i < count - 1) {
			if (*p_end != sep) return FALSE;
			p_text = p_end + 1;
		}
		else if (*p_end) return FALSE;
	}
	return TRUE;
}

// Writes the date and time to the chip and to pif_datetime.
static BOOL _setDateTime(const PifDateTime *p_datetime)
{
	if (!pifRtc_SetDateTime(&g_ds1302.parent, p_datetime)) return FALSE;
	return pif_SetDateTime(p_datetime);
}

static int _cmdTime(int argc, char *argv[])
{
	PifDateTime datetime;
	uint16_t date[3], time[2];

	if (argc == 0) {
		_show();
		return PIF_LOG_CMD_NO_ERROR;
	}
	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;

	if (!_parseFields(argv[0], '-', date, 3) || !_parseFields(argv[1], ':', time, 2)) return PIF_LOG_CMD_INVALID_ARG;
	if (date[0] < 2000 || date[0] > 2099) return PIF_LOG_CMD_INVALID_ARG;
	datetime.year = date[0] - 2000;
	datetime.month = date[1];
	datetime.day = date[2];
	datetime.hour = time[0];
	datetime.minute = time[1];
	datetime.second = 0;
	datetime.millisecond = 0;
	if (!_setDateTime(&datetime)) return PIF_LOG_CMD_INVALID_ARG;

	_show();
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdShow(int argc, char *argv[])
{
	if (argc == 0) return PIF_LOG_CMD_TOO_FEW_ARGS;

	if (!strcmp(argv[0], "on")) {
		pifTask_Resume(s_p_task_show);
	}
	else if (!strcmp(argv[0], "off")) {
		pifTask_Pause(s_p_task_show);
	}
	else return PIF_LOG_CMD_INVALID_ARG;
	return PIF_LOG_CMD_NO_ERROR;
}

static const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "time", _cmdTime, "Print or set the time", "[YYYY-MM-DD hh:mm]" },
	{ "show", _cmdShow, "Show the time every second", "on | off" },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
	PifRtc *p_rtc = &g_ds1302.parent;
	PifDateTime build, rtc;
	char text[20];
	uint8_t boot_count;
	BOOL running, set = FALSE;

    if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) return FALSE;							// 32bytes

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;											// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           ex_rtc_ds1302          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");

	// A new chip either stays halted until the time is written, or runs from 2000-01-01 or from whatever its
	// registers held. Any time before the build cannot be right, so it is set to the build time.
	_getBuildTime(&build);
	if (!pifDs1302_IsRunning(&g_ds1302, &running)) return FALSE;
	if (!running || !pifRtc_GetDateTime(p_rtc, &rtc) || pifRtc_ToSeconds(&rtc) < pifRtc_ToSeconds(&build)) {
		if (!pifRtc_SetDateTime(p_rtc, &build)) return FALSE;
		_format(text, &build);
		pifLog_Printf(LT_INFO, "RTC was not set: set to the build time %s", text);
		set = TRUE;
	}

	if (!pifRtc_LoadDateTime(p_rtc)) {
		// No chip, or wrong wiring: the lines read all ones, as a halted clock.
		pifLog_Printf(LT_ERROR, "RTC read failed: E=%d. Check the wiring.", pif_error);
		return TRUE;
	}

	if (!pifDs1302_ReadRam(&g_ds1302, RAM_BOOT_COUNT, &boot_count, 1)) return FALSE;
	if (set) boot_count = 0;
	boot_count++;
	if (!pifDs1302_WriteRam(&g_ds1302, RAM_BOOT_COUNT, &boot_count, 1)) return FALSE;
	pifLog_Printf(LT_INFO, "Boot count in the RAM of the DS1302: %u", boot_count);

	if (!pifRtc_StartSync(p_rtc, SYNC_PERIOD, _evtRtcSync, NULL, NULL)) return FALSE;

	s_p_task_show = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, SHOW_PERIOD * 1000UL, _taskShow, NULL, TRUE);
	if (!s_p_task_show) return FALSE;
	s_p_task_show->name = "Show";

	pifLog_Print(LT_INFO, "Set the time with: time YYYY-MM-DD hh:mm");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
