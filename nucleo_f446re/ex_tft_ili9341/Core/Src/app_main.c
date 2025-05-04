#include "linker.h"

#include "filter/pif_noise_filter_int16.h"


PifLed g_led_l;
PifIli9341 g_ili9341;
PifTimerManager g_timer_1ms;
#if LCD_TYPE == LCD_3_2_INCH
	PifSpiPort g_spi_port;
	PifTsc2046 g_tsc2046;
#endif

static PifTftLcdRotation rotation = TLR_0_DEGREE;
static uint8_t backlight = 50;
#if LCD_TYPE != LCD_2_2_INCH_SPI
	static BOOL draw = FALSE;
#endif

static PifTask* p_task = NULL;

static int _CmdFillRect(int argc, char *argv[]);
static int _CmdRotation(int argc, char *argv[]);
static int _CmdBackLight(int argc, char *argv[]);
#if LCD_TYPE != LCD_2_2_INCH_SPI
	static int _CmdTouchCalibration(int argc, char *argv[]);
	static int _CmdClear(int argc, char *argv[]);
	static int _CmdDraw(int argc, char *argv[]);
#endif

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fr", _CmdFillRect, "Fill Rect", NULL },
	{ "rot", _CmdRotation, "Change Rotation", NULL },
	{ "bl", _CmdBackLight, "Change Back Light", NULL },
#if LCD_TYPE != LCD_2_2_INCH_SPI
	{ "tc", _CmdTouchCalibration, "Touch Calibration", NULL },
	{ "clear", _CmdClear, "Clear", NULL },
	{ "draw", _CmdDraw, "Drawing", NULL },
#endif

	{ NULL, NULL, NULL, NULL }
};


static int _CmdFillRect(int argc, char *argv[])
{
	p_task->pause ^= 1;
	pifLog_Printf(LT_NONE, "  File Rect=%u\n", !p_task->pause);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdRotation(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Rotation=%u\n", rotation);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		int rot = atoi(argv[0]);
		if (rot >= TLR_0_DEGREE && rot <= TLR_270_DEGREE) {
			rotation = (PifTftLcdRotation)rot;
			pifIli9341_SetRotation(&g_ili9341.parent, rotation);
			return PIF_LOG_CMD_NO_ERROR;
		}
		return PIF_LOG_CMD_INVALID_ARG;
	}

	return PIF_LOG_CMD_TOO_FEW_ARGS;
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
		if (g_ili9341.parent.act_backlight) (*g_ili9341.parent.act_backlight)(backlight);
		pifLog_Printf(LT_NONE, "  BackLight=%u\n", backlight);
		return PIF_LOG_CMD_NO_ERROR;
	}

	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

#if LCD_TYPE != LCD_2_2_INCH_SPI

static int _CmdTouchCalibration(int argc, char *argv[])
{
#if LCD_TYPE == LCD_3_2_INCH
	if (pifTouchScreen_Calibration(&g_tsc2046.parent)) {
		pifLog_Printf(LT_INFO, "Touch calibration is success");
	}
	else {
		pifLog_Printf(LT_INFO, "Touch calibration is failure");
	}
#endif
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdClear(int argc, char *argv[])
{
	pifIli9341_DrawFillRect(&g_ili9341.parent, 0, 0, g_ili9341.parent._width - 1, g_ili9341.parent._height - 1, BLACK);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDraw(int argc, char *argv[])
{
	draw ^= 1;
	pifLog_Printf(LT_NONE, "  Draw=%u\n", draw);
	return PIF_LOG_CMD_NO_ERROR;
}

#endif

static uint32_t _taskFillScreen(PifTask *pstTask)
{
	static int step = 0;

	(void)pstTask;

	switch (step) {
	case 0:
		pifIli9341_DrawFillRect(&g_ili9341.parent, 5, 10, g_ili9341.parent._width - 14, g_ili9341.parent._height - 19, RED);
		step++;
		break;

	case 1:
		pifIli9341_DrawFillRect(&g_ili9341.parent, 10, 20, g_ili9341.parent._width - 29, g_ili9341.parent._height - 39, GREEN);
		step++;
		break;

	case 2:
		pifIli9341_DrawFillRect(&g_ili9341.parent, 15, 30, g_ili9341.parent._width - 44, g_ili9341.parent._height - 59, BLUE);
		step++;
		break;

	case 3:
		pifIli9341_DrawFillRect(&g_ili9341.parent, 20, 40, g_ili9341.parent._width - 59, g_ili9341.parent._height - 79, WHITE);
		step++;
		break;

	case 4:
		pifIli9341_DrawFillRect(&g_ili9341.parent, 0, 0, g_ili9341.parent._width - 1, g_ili9341.parent._height - 1, BLACK);
		step = 0;
		break;
	}
	pifLog_Printf(LT_NONE, "  Color=%u\n", step);
	return 0;
}

#if LCD_TYPE != LCD_2_2_INCH_SPI

static void _evtTouchData(int16_t x, int16_t y)
{
    static int time = 0;

    if (draw) {
		pifIli9341_DrawPixel(&g_ili9341.parent, x, y, WHITE);
	}

    if (time) time--;
	else {
		pifLog_Printf(LT_INFO, "TS: x=%u, y=%u", x, y);
		time = 50;
	}
}

#endif

BOOL appSetup()
{
    static PifNoiseFilterManager s_filter;
    PifNoiseFilter* p_filter[2];
    int line;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }				// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) { line = __LINE__; goto fail; }	// 32bytes

	if (!pifNoiseFilterManager_Init(&s_filter, 2)) { line = __LINE__; goto fail; }
	p_filter[0] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);								// touch x
	if (!p_filter[0]) { line = __LINE__; goto fail; }
	p_filter[1] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);								// touch y
	if (!p_filter[1]) { line = __LINE__; goto fail; }

#if LCD_TYPE == LCD_3_2_INCH
	g_tsc2046.parent.evt_touch_data = _evtTouchData;
	if (!pifTouchScreen_AttachFilter(&g_tsc2046.parent, p_filter[0], p_filter[1])) { line = __LINE__; goto fail; }
	if (!pifTouchScreen_Start(&g_tsc2046.parent, NULL)) { line = __LINE__; goto fail; }
#endif

	pifIli9341_SetRotation(&g_ili9341.parent, TLR_0_DEGREE);

    p_task = pifTaskManager_Add(TM_PERIOD, 2000000, _taskFillScreen, NULL, FALSE);			// 2000ms
    if (!p_task) { line = __LINE__; goto fail; }
    p_task->name = "FillRect";

	pifIli9341_DrawFillRect(&g_ili9341.parent, 0, 0, g_ili9341.parent._width - 1, g_ili9341.parent._height - 1, BLACK);
    return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
