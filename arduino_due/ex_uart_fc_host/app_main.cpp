#include "linker.h"

#include <stdlib.h>


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_host;

static int s_step = -1;

static int _CmdFlowControl(int argc, char *argv[]);
static int _CmdSendData(int argc, char *argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fc", _CmdFlowControl, "Set and print flow control", NULL },
	{ "send", _CmdSendData, "Send data", NULL },

	{ NULL, NULL, NULL, NULL }
};


static void _evtUartTxFlowState(void* p_client, SWITCH state)
{
	(void)p_client;

	pifLog_Printf(LT_INFO, "Tx Flow State=%d Step=%d", state, s_step);
}

static int _CmdFlowControl(int argc, char *argv[])
{
	const char* name[] = { "None", "Software", "hardware" };

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Flow Control=%s\n", name[g_uart_host._flow_control]);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "sw")) {
			pifUart_SetFlowControl(&g_uart_host, UFC_SOFTWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "hw")) {
			pifUart_SetFlowControl(&g_uart_host, UFC_HARDWARE, _evtUartTxFlowState);
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
	s_step = 0;
	return PIF_LOG_CMD_NO_ERROR;
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x", ch);
}

BOOL _evtUartSending(void* p_client, PifActUartSendData act_send_data)
{
	uint8_t text[3];
	static int p = 0;

	if (s_step < 0) return FALSE;
	if (!g_uart_host._fc_state) return FALSE;

	if (p < 26) {
		text[0] = 'a' + p;
		text[1] = 'a' + p + 1;
	}
	else {
		text[0] = '\r';
		text[0] = '\n';
	}

	if (!(*act_send_data)(&g_uart_host, text, 2)) {
		pifLog_Print(LT_INFO, "Not send\n");
		return FALSE;
	}
	p += 2;
	if (p >= 28) {
		p = 0;
		s_step++;
		if (s_step == 100) s_step = -1;
	}
	return TRUE;
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;
    pifLog_AttachEvent(_evtLogControlChar);

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;							// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifUart_AttachClient(&g_uart_host, NULL, NULL, _evtUartSending);
	return TRUE;
}
