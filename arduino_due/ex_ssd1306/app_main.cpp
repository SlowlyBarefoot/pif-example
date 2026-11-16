#include "app_port.h"


// 화면 구성: 제목과 동작 시간, 가로줄, 공이 튀는 상자, 아래쪽에 막대 그래프
#define BOX_Y1					12
#define BOX_Y2					47
#define BALL_RADIUS				4

#define BAR_X1					58
#define BAR_X2					(SSD1306_WIDTH - 1)
#define BAR_Y1					54
#define BAR_Y2					62

#define ANIMATION_PERIOD_US		50000UL		// 50ms
#define STATUS_PERIOD_US		500000UL	// 500ms


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifSsd1306 g_ssd1306;
PifTimerManager g_timer_1ms;

static int16_t s_ball_x, s_ball_y;
static int16_t s_ball_dx = 2, s_ball_dy = 1;
static uint8_t s_load = 0;
static BOOL s_load_up = TRUE;
static BOOL s_inverse = FALSE;

static int _CmdContrast(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry);
static int _CmdInverse(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry);
static int _CmdFlip(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry);
static int _CmdDisplay(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "ct", _CmdContrast, "Change contrast (0~255)", NULL },
	{ "inv", _CmdInverse, "Toggle inverse", NULL },
	{ "flip", _CmdFlip, "Flip the picture (hor ver)", NULL },
	{ "disp", _CmdDisplay, "Turn the display on(1) or off(0)", NULL },

	{ NULL, NULL, NULL, NULL }
};


static int _CmdContrast(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	int contrast;

	(void)p_entry;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;

	contrast = atoi(argv[0]);
	if (contrast < 0 || contrast > 255) return PIF_LOG_CMD_INVALID_ARG;
	if (!pifSsd1306_SetContrast(&g_ssd1306, (uint8_t)contrast)) return PIF_LOG_CMD_USER_ERROR;

	pifLog_Printf(LT_NONE, "  Contrast=%d\n", contrast);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdInverse(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	(void)argc;
	(void)argv;
	(void)p_entry;

	// 패널이 보여주는 방식만 바뀐다. 프레임 버퍼는 그대로라 다시 그릴 필요가 없다.
	s_inverse = !s_inverse;
	if (s_inverse) {
		if (!pifSsd1306_Inverse(&g_ssd1306)) return PIF_LOG_CMD_USER_ERROR;
	}
	else {
		if (!pifSsd1306_NoInverse(&g_ssd1306)) return PIF_LOG_CMD_USER_ERROR;
	}

	pifLog_Printf(LT_NONE, "  Inverse=%u\n", s_inverse);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdFlip(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	BOOL flip_hor, flip_ver;

	(void)p_entry;

	if (argc < 2) return PIF_LOG_CMD_TOO_FEW_ARGS;

	flip_hor = atoi(argv[0]) ? TRUE : FALSE;
	flip_ver = atoi(argv[1]) ? TRUE : FALSE;
	if (!pifSsd1306_SetFlip(&g_ssd1306, flip_hor, flip_ver)) return PIF_LOG_CMD_USER_ERROR;

	pifLog_Printf(LT_NONE, "  Flip=%u,%u\n", flip_hor, flip_ver);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdDisplay(int argc, char *argv[], const struct StPifLogCmdEntry *p_entry)
{
	BOOL on;

	(void)p_entry;

	if (argc < 1) return PIF_LOG_CMD_TOO_FEW_ARGS;

	on = atoi(argv[0]) ? TRUE : FALSE;
	if (on) {
		if (!pifSsd1306_Display(&g_ssd1306)) return PIF_LOG_CMD_USER_ERROR;
	}
	else {
		if (!pifSsd1306_NoDisplay(&g_ssd1306)) return PIF_LOG_CMD_USER_ERROR;
	}

	pifLog_Printf(LT_NONE, "  Display=%u\n", on);
	return PIF_LOG_CMD_NO_ERROR;
}

/**
 * @fn _drawFrame
 * @brief 움직이지 않는 부분을 한 번만 그린다.
 */
static void _drawFrame(PifGraphics *p_gfx)
{
	pifGraphics_DrawClear(p_gfx, SSD1306_COLOR_BLACK);

	pifGraphics_SetCursor(p_gfx, 0, 0);
	pifGraphics_DrawString(p_gfx, "SSD1306 OLED", SSD1306_COLOR_WHITE);
	pifGraphics_DrawHorLine(p_gfx, 0, 9, SSD1306_WIDTH, SSD1306_COLOR_WHITE);

	pifGraphics_DrawRect(p_gfx, 0, BOX_Y1, SSD1306_WIDTH - 1, BOX_Y2, SSD1306_COLOR_WHITE);

	pifGraphics_SetCursor(p_gfx, 0, BAR_Y1);
	pifGraphics_DrawString(p_gfx, "Load", SSD1306_COLOR_WHITE);
	pifGraphics_DrawRect(p_gfx, BAR_X1, BAR_Y1, BAR_X2, BAR_Y2, SSD1306_COLOR_WHITE);
}

/**
 * @fn _taskAnimation
 * @brief 공이 있던 자리를 지우고 새 자리에 그린다.
 *
 * 전체를 지우고 다시 그리지 않으므로 바뀐 페이지의 바뀐 칸만 패널로 간다. 한 장을 통째로 보내면 1KB인데
 * 이 방식은 공이 지나간 몇십 바이트로 끝난다.
 */
static uint32_t _taskAnimation(PifTask *p_task)
{
	PifGraphics *p_gfx = pifSsd1306_Graphics(&g_ssd1306);
	int16_t left = 1 + BALL_RADIUS, right = SSD1306_WIDTH - 2 - BALL_RADIUS;
	int16_t top = BOX_Y1 + 1 + BALL_RADIUS, bottom = BOX_Y2 - 1 - BALL_RADIUS;

	(void)p_task;

	pifGraphics_DrawFillCircle(p_gfx, s_ball_x, s_ball_y, BALL_RADIUS, SSD1306_COLOR_BLACK);

	s_ball_x += s_ball_dx;
	s_ball_y += s_ball_dy;
	if (s_ball_x <= left) {
		s_ball_x = left;
		s_ball_dx = -s_ball_dx;
	}
	else if (s_ball_x >= right) {
		s_ball_x = right;
		s_ball_dx = -s_ball_dx;
	}
	if (s_ball_y <= top) {
		s_ball_y = top;
		s_ball_dy = -s_ball_dy;
	}
	else if (s_ball_y >= bottom) {
		s_ball_y = bottom;
		s_ball_dy = -s_ball_dy;
	}

	// 튀는 범위를 반지름만큼 좁혀 두었으므로 공이 상자 테두리를 건드리지 않는다. 테두리는 처음 그린 채로 둔다.
	pifGraphics_DrawFillCircle(p_gfx, s_ball_x, s_ball_y, BALL_RADIUS, SSD1306_COLOR_WHITE);

	if (!pifSsd1306_Update(&g_ssd1306)) pifLog_Printf(LT_ERROR, "Update:%d", pif_error);
	return 0;
}

/**
 * @fn _taskStatus
 * @brief 동작 시간과 막대 그래프를 고쳐 쓴다.
 */
static uint32_t _taskStatus(PifTask *p_task)
{
	PifGraphics *p_gfx = pifSsd1306_Graphics(&g_ssd1306);
	uint16_t inside = BAR_X2 - BAR_X1 - 1;
	uint16_t fill = inside * s_load / 100;

	(void)p_task;

	// 글자는 칸 전체를 그리므로 앞서 쓴 숫자를 따로 지우지 않아도 덮인다.
	pifGraphics_SetCursor(p_gfx, 98, 0);
	pifGraphics_Printf(p_gfx, SSD1306_COLOR_WHITE, "%4lu", (unsigned long)(pif_cumulative_timer1ms / 1000));

	pifGraphics_SetCursor(p_gfx, 30, BAR_Y1);
	pifGraphics_Printf(p_gfx, SSD1306_COLOR_WHITE, "%3d%%", s_load);

	if (fill) pifGraphics_DrawFillRect(p_gfx, BAR_X1 + 1, BAR_Y1 + 1, BAR_X1 + fill, BAR_Y2 - 1,
			SSD1306_COLOR_WHITE);
	if (fill < inside) pifGraphics_DrawFillRect(p_gfx, BAR_X1 + 1 + fill, BAR_Y1 + 1, BAR_X2 - 1, BAR_Y2 - 1,
			SSD1306_COLOR_BLACK);

	if (s_load_up) {
		s_load += 5;
		if (s_load >= 100) s_load_up = FALSE;
	}
	else {
		s_load -= 5;
		if (!s_load) s_load_up = TRUE;
	}

	if (!pifSsd1306_Update(&g_ssd1306)) pifLog_Printf(LT_ERROR, "Update:%d", pif_error);
	return 0;
}

BOOL appSetup()
{
	PifGraphics *p_gfx = pifSsd1306_Graphics(&g_ssd1306);
	PifTask *p_task;

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;								// 500ms
	g_led_l._p_task->name = "LedL";
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	if (!pifLog_UseCommand(32, c_psCmdTable, "\nDebug> ")) return FALSE;					// 32bytes

	s_ball_x = SSD1306_WIDTH / 2;
	s_ball_y = (BOX_Y1 + BOX_Y2) / 2;

	_drawFrame(p_gfx);
	if (!pifSsd1306_Update(&g_ssd1306)) return FALSE;

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, ANIMATION_PERIOD_US, _taskAnimation, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "Ball";

	p_task = pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, STATUS_PERIOD_US, _taskStatus, NULL, TRUE);
	if (!p_task) return FALSE;
	p_task->name = "Status";

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_ssd1306            ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE,
			pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;
}
