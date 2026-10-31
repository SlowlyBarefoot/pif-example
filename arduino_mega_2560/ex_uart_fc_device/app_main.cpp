#include "app_port.h"

#include <stdlib.h>
#include <string.h>


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

static BOOL _evtUartParsing(void *p_client, PifActTransportReceive act_receive_data)
{
	uint8_t data[2] = { 0, 0 };

	(void)p_client;

	if (!s_count_down) return TRUE;
	s_count_down = 0;

	if ((*act_receive_data)(&g_uart_device.parent, data, 1)) {
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

#ifdef PIF_USE_COMM_TAP

#define TAP_LAST_SIZE			16

// What the tap of the UART saw for each PifTapDir: TD_RX, TD_TX and TD_RX_LOST.
// Each direction is written from one context only, so the tap needs no lock.
static struct {
	uint32_t count;
	uint8_t last[TAP_LAST_SIZE];		// The last bytes, a ring
	uint8_t pos;
} s_tap[3];

// It may be called from an interrupt, so it only counts and keeps the last bytes.
static void _evtUartTap(PifUart *p_uart, PifTapDir dir, const uint8_t *p_data, uint16_t length)
{
	(void)p_uart;

	s_tap[dir].count += length;
	while (length--) {
		s_tap[dir].last[s_tap[dir].pos] = *p_data++;
		s_tap[dir].pos = (s_tap[dir].pos + 1) % TAP_LAST_SIZE;
	}
}

static int _CmdTap(int argc, char *argv[])
{
	const char *name[] = { "Rx", "Tx", "RxLost" };
	uint32_t i, j, n;

	if (argc > 0) {
		if (strcmp(argv[0], "clear")) return PIF_LOG_CMD_INVALID_ARG;
		memset(s_tap, 0, sizeof(s_tap));
		return PIF_LOG_CMD_NO_ERROR;
	}
	for (i = 0; i < 3; i++) {
		pifLog_Printf(LT_NONE, "  %s=%lu:", name[i], (unsigned long)s_tap[i].count);
		n = s_tap[i].count < TAP_LAST_SIZE ? s_tap[i].count : TAP_LAST_SIZE;
		for (j = TAP_LAST_SIZE - n; j < TAP_LAST_SIZE; j++) {
			pifLog_Printf(LT_NONE, " %2X", s_tap[i].last[(s_tap[i].pos + j) % TAP_LAST_SIZE]);
		}
		pifLog_Print(LT_NONE, "\n");
	}
	return PIF_LOG_CMD_NO_ERROR;
}

#endif

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fc", _CmdFlowControl, "Set and print flow control", "[no | sw | hw]" },
#ifdef PIF_USE_COMM_TAP
	{ "tap", _CmdTap, "Print the tapped bytes, or clear them", "[clear]" },
#endif

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
    if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) return FALSE;									// 32bytes
    pifLog_AttachEvent(_evtLogControlChar);

#ifdef PIF_USE_COMM_TAP
	g_uart_device.evt_tap = _evtUartTap;
#endif

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;												// 500ms
	g_led_l._p_task->name = "LedL";
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifTransport_AttachClient(&g_uart_device.parent, NULL, _evtUartParsing, NULL, NULL);

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 5000, _taskSlowProcess, NULL, TRUE)) return FALSE;	// 5ms

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         ex_uart_fc_device        ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;
}
