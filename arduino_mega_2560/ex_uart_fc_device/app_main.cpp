#include "linker.h"

#include <stdlib.h>


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_device;

static int s_count_down = 0;


static void _evtUartTxFlowState(void *p_client, SWITCH state)
{
	(void)p_client;

	pifLog_Printf(LT_INFO, "Tx Flow State=%d", state);
}

static int _CmdFlowControl(int argc, char *argv[])
{
	const char *name[] = { "None", "", "Software", "Hardware", "Software", "Hardware" };
	int limit;

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Flow Control=%s\n", name[g_uart_device._flow_control]);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "sw")) {
			pifUart_SetFlowControl(&g_uart_device, UFC_DEVICE_SOFTWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "hw")) {
			pifUart_SetFlowControl(&g_uart_device, UFC_DEVICE_HARDWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "no")) {
			pifUart_ResetFlowControl(&g_uart_device);
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		if (argc > 1) {
			limit = atoi(argv[1]);
			if (limit >= 20 && limit <= 80) {
				g_uart_device.fc_limit = limit;
				pifLog_Printf(LT_INFO, "Flow Control: Limit=%d", limit);
			}
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x\n", ch);
}

static BOOL _evtUartParsing(void *p_client, PifActUartReceiveData act_receive_data)
{
	uint8_t data[2] = { 0, 0 };

	(void)p_client;

	if (!s_count_down) return TRUE;
	s_count_down = 0;

	if ((*act_receive_data)(&g_uart_device, data, 1)) {
		pifLog_Print(LT_NONE, (char *)data);
		return TRUE;
	}
	return FALSE;
}

static uint32_t _taskSlowProcess(PifTask *p_task)
{
	(void)p_task;

	s_count_down = 1;
    return 0;
}

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fc", _CmdFlowControl, "Set and print flow control", NULL },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
    int line;

    if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) { line = __LINE__; goto fail; }									// 32bytes
    pifLog_AttachEvent(_evtLogControlChar);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }												// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifUart_AttachClient(&g_uart_device, NULL, _evtUartParsing, NULL);

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 5000, _taskSlowProcess, NULL, TRUE)) { line = __LINE__; goto fail; }	// 5ms
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
