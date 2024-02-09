#include "linker.h"

#include "filter/pif_noise_filter_int16.h"


PifLed g_led_l;
PifSsd1963 g_ssd1963;
PifTimerManager g_timer_1ms;
PifSpiPort g_spi_port;
PifTsc2046 g_tsc2046;

static PifTftLcdRotation rotation = TLR_0_DEGREE;
static uint8_t backlight = 50;
static BOOL draw = FALSE;

static PifTask* p_task = NULL;

static int _CmdFillRect(int argc, char *argv[]);
static int _CmdBackLight(int argc, char *argv[]);
static int _CmdTouchCalibration(int argc, char *argv[]);
static int _CmdClear(int argc, char *argv[]);
static int _CmdDraw(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fr", _CmdFillRect, "Fill Rect", NULL },
	{ "bl", _CmdBackLight, "Change Back Light", NULL },
	{ "tc", _CmdTouchCalibration, "Touch Calibration", NULL },
	{ "clear", _CmdClear, "Clear", NULL },
	{ "draw", _CmdDraw, "Drawing", NULL },

	{ NULL, NULL, NULL, NULL }
};


static int _CmdFillRect(int argc, char *argv[])
{
	p_task->pause ^= 1;
	pifLog_Printf(LT_NONE, "  File Rect=%u\n", !p_task->pause);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdBackLight(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  BackLight=%u\n", backlight);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		int bl = atoi(argv[0]);
		if (bl < 0) backlight = 0;
		else if (bl > 100) backlight = 100;
		else backlight = bl;
		if (g_ssd1963.parent.act_backlight) (*g_ssd1963.parent.act_backlight)(backlight);
		pifLog_Printf(LT_NONE, "  BackLight=%u\n", backlight);
		return PIF_LOG_CMD_NO_ERROR;
	}

	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdTouchCalibration(int argc, char *argv[])
{
	if (pifTouchScreen_Calibration(&g_tsc2046.parent)) {
		pifLog_Printf(LT_INFO, "Touch calibration is success");
	}
	else {
		pifLog_Printf(LT_INFO, "Touch calibration is failure");
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdClear(int argc, char *argv[])
{
	pifSsd1963_DrawFillRect(&g_ssd1963.parent, 0, 0, g_ssd1963.parent._width - 1, g_ssd1963.parent._height - 1, BLACK);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDraw(int argc, char *argv[])
{
	draw ^= 1;
	pifLog_Printf(LT_NONE, "  Draw=%u\n", draw);
	return PIF_LOG_CMD_NO_ERROR;
}

static uint16_t _taskFillScreen(PifTask *pstTask)
{
	static int step = 0;

	(void)pstTask;

	switch (step) {
	case 0:
		pifSsd1963_DrawFillRect(&g_ssd1963.parent, 5, 10, g_ssd1963.parent._width - 14, g_ssd1963.parent._height - 19, RED);
		step++;
		break;

	case 1:
		pifSsd1963_DrawFillRect(&g_ssd1963.parent, 10, 20, g_ssd1963.parent._width - 29, g_ssd1963.parent._height - 39, GREEN);
		step++;
		break;

	case 2:
		pifSsd1963_DrawFillRect(&g_ssd1963.parent, 15, 30, g_ssd1963.parent._width - 44, g_ssd1963.parent._height - 59, BLUE);
		step++;
		break;

	case 3:
		pifSsd1963_DrawFillRect(&g_ssd1963.parent, 20, 40, g_ssd1963.parent._width - 59, g_ssd1963.parent._height - 79, WHITE);
		step++;
		break;

	case 4:
		pifSsd1963_DrawFillRect(&g_ssd1963.parent, 0, 0, g_ssd1963.parent._width - 1, g_ssd1963.parent._height - 1, BLACK);
		step = 0;
		break;
	}
	pifLog_Printf(LT_NONE, "  Color=%u\n", step);
	return 0;
}

static void _evtTouchData(int16_t x, int16_t y)
{
    static int time = 0;

    if (draw) {
		pifSsd1963_DrawPixel(&g_ssd1963.parent, x, y, WHITE);
	}

    if (time) time--;
	else {
		pifLog_Printf(LT_INFO, "TS: x=%u, y=%u", x, y);
		time = 50;
	}
}

BOOL appSetup()
{
    static PifNoiseFilterManager s_filter;
    PifNoiseFilter* p_filter[2];
    int line;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }			// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) { line = __LINE__; goto fail; }

	if (!pifNoiseFilterManager_Init(&s_filter, 2)) { line = __LINE__; goto fail; }
	p_filter[0] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);							// touch x
	if (!p_filter[0]) { line = __LINE__; goto fail; }
	p_filter[1] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);							// touch y
	if (!p_filter[1]) { line = __LINE__; goto fail; }

	g_tsc2046.parent.evt_touch_data = _evtTouchData;
	if (!pifTouchScreen_AttachFilter(&g_tsc2046.parent, p_filter[0], p_filter[1])) { line = __LINE__; goto fail; }
	if (!pifTouchScreen_Start(&g_tsc2046.parent, NULL)) { line = __LINE__; goto fail; }

	p_task = pifTaskManager_Add(TM_PERIOD_MS, 2000, _taskFillScreen, NULL, FALSE);		// 2000ms
    if (!p_task) { line = __LINE__; goto fail; }
    p_task->name = "FillRect";

	pifSsd1963_DrawFillRect(&g_ssd1963.parent, 0, 0, g_ssd1963.parent._width - 1, g_ssd1963.parent._height - 1, BLACK);
    return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
