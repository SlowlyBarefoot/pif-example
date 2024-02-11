#include "linker.h"

#include "filter/pif_noise_filter_int16.h"

#include <lvgl.h>


PifLed g_led_l;
PifIli9341 g_ili9341;
PifTimerManager g_timer_1ms;
PifSpiPort g_spi_port;
PifTsc2046 g_tsc2046;

static uint8_t backlight = 50;

static PifTask* p_task = NULL;

static lv_disp_drv_t disp_drv;
static lv_disp_t *p_disp;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ ILI9341_HEIGHT * ILI9341_WIDTH / 10 ];

static int _CmdBackLight(int argc, char *argv[]);
static int _CmdTouchCalibration(int argc, char *argv[]);
static int _CmdExample(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "bl", _CmdBackLight, "Change Back Light", NULL },
	{ "tc", _CmdTouchCalibration, "Touch Calibration", NULL },
	{ "ex", _CmdExample, "Example", NULL },

	{ NULL, NULL, NULL, NULL }
};


static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
    	pifLog_Print(LT_INFO, "Clicked");
    }
    else if (code == LV_EVENT_VALUE_CHANGED) {
    	pifLog_Print(LT_INFO, "Toggled");
    }
}

static void lv_example_btn_1(void)
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
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

static int _CmdTouchCalibration(int argc, char *argv[])
{
	if (pifTouchScreen_Calibration(&g_tsc2046.parent)) {
		pifLog_Printf(LT_INFO, "Touch calibration is success");

		lv_obj_t * my_rect = lv_obj_create(lv_scr_act());
	    lv_obj_set_size(my_rect , ILI9341_HEIGHT, ILI9341_WIDTH);
	    lv_obj_set_pos(my_rect , 0, 0);
	    lv_obj_set_style_bg_color(my_rect , (lv_color_t)LV_COLOR_MAKE(255, 255, 255), 0);
	}
	else {
		pifLog_Printf(LT_INFO, "Touch calibration is failure");
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _CmdExample(int argc, char *argv[])
{
	pifLog_Printf(LT_INFO, "Example:1");
    lv_example_btn_1();
	return PIF_LOG_CMD_NO_ERROR;
}

static uint16_t _taskLvglTimerHandler(PifTask *p_task)
{
	(void)p_task;

    lv_timer_handler(); /* let the GUI do its work */
	return 0;
}

/* Display flushing */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color)
{
    pifIli9341_DrawArea(&g_ili9341.parent, area->x1, area->y1, area->x2, area->y2, (PifColor*)&color->full);

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;

    bool touched = g_tsc2046.parent._pressure;
    touchX = g_tsc2046.parent._x;
    touchY = g_tsc2046.parent._y;

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    }
    else {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

		pifLog_Printf(LT_INFO, "Touch: %d x %d", touchX, touchY);
    }
}

BOOL appSetup()
{
    static PifNoiseFilterManager s_filter;
    PifNoiseFilter* p_filter[2];
    static lv_indev_drv_t indev_drv;
    lv_obj_t *my_rect;
    lv_obj_t *label;
    char LVGL_Arduino[30];
    int line;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }			// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) { line = __LINE__; goto fail; }

	if (!pifNoiseFilterManager_Init(&s_filter, 2)) { line = __LINE__; goto fail; }
	p_filter[0] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);							// touch x
	if (!p_filter[0]) { line = __LINE__; goto fail; }
	p_filter[1] = pifNoiseFilterInt16_AddAverage(&s_filter, 5);							// touch y
	if (!p_filter[1]) { line = __LINE__; goto fail; }

	if (!pifTouchScreen_AttachFilter(&g_tsc2046.parent, p_filter[0], p_filter[1])) { line = __LINE__; goto fail; }
	if (!pifTouchScreen_Start(&g_tsc2046.parent, NULL)) { line = __LINE__; goto fail; }

	pifIli9341_SetRotation(&g_ili9341.parent, TLR_270_DEGREE);

    lv_init();

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, ILI9341_WIDTH * ILI9341_HEIGHT / 10 );

    /*Initialize the display*/
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = ILI9341_HEIGHT;
    disp_drv.ver_res = ILI9341_WIDTH;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    p_disp = lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    my_rect = lv_obj_create(lv_scr_act());
    lv_obj_set_size(my_rect, ILI9341_HEIGHT, ILI9341_WIDTH);
    lv_obj_set_pos(my_rect, 0, 0);
    lv_obj_set_style_bg_color(my_rect, (lv_color_t)LV_COLOR_MAKE(255, 0, 0), 0);

    /* Create simple label */
    pif_Printf(LVGL_Arduino, 30, "Hello Arduino! v%d.%d.%d", lv_version_major(), lv_version_minor(), lv_version_patch());
    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, LVGL_Arduino);
    lv_obj_set_style_text_color(label, (lv_color_t)LV_COLOR_MAKE(0, 0, 0), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    p_task = pifTaskManager_Add(TM_PERIOD_MS, 5, _taskLvglTimerHandler, NULL, TRUE);	// 5ms
    if (!p_task) { line = __LINE__; goto fail; }
    p_task->name = "LVGL";
    return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
