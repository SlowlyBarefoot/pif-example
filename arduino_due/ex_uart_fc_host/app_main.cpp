#include "linker.h"

#include <stdio.h>
#include <stdlib.h>


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_host;
PifTask* p_task = NULL;


static void _evtUartTxFlowState(void *p_client, SWITCH state)
{
	(void)p_client;

	pifLog_Printf(LT_INFO, "Tx Flow State=%d", state);
}

static int _CmdFlowControl(int argc, char *argv[])
{
	const char *name[] = { "None", "", "Software", "Hardware", "Software", "Hardware" };

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Flow Control=%s\n", name[g_uart_host._flow_control]);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "sw")) {
			pifUart_SetFlowControl(&g_uart_host, UFC_HOST_SOFTWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "hw")) {
			pifUart_SetFlowControl(&g_uart_host, UFC_HOST_HARDWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "no")) {
			pifUart_ResetFlowControl(&g_uart_host);
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdSendData(int argc, char *argv[])
{
	p_task->pause = FALSE;
	return PIF_LOG_CMD_NO_ERROR;
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x", ch);
}

static uint16_t _taskSendMessage(PifTask *p_task)
{
	char message[35];
	int length, pos = 0, len;
	static int s_step = 1;

	pifLog_Printf(LT_INFO, "%d", s_step);
	sprintf(message, "%d: abcdefghijklmnopqrstuvwxyz\r\n", s_step);
	length = strlen(message);
	while (1) {
		if (g_uart_host._fc_state) {
			len = pifUart_SendTxData(&g_uart_host, (uint8_t *)message + pos, length - pos);
			if (pos + len < length) pos += len; else break;
		}
		pifTaskManager_Yield();
	}

	s_step++;
	if (s_step == 100) {
		p_task->pause = TRUE;
		s_step = 0;
	}
	return 0;
}

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fc", _CmdFlowControl, "Set and print flow control", NULL },
	{ "send", _CmdSendData, "Send data", NULL },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
	int line;

	if (!pifLog_UseCommand(c_cmd_table, "\nDebug> ")) { line = __LINE__; goto fail; }
	pifLog_AttachEvent(_evtLogControlChar);

	g_uart_host._p_task->pause = FALSE;

	if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }			// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	p_task = pifTaskManager_Add(TM_PERIOD_MS, 5, _taskSendMessage, NULL, FALSE);		// 5ms
	if (!p_task) { line = __LINE__; goto fail; }
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
