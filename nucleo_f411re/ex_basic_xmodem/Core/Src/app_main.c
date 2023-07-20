#include "linker.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "interpreter/pif_basic.h"
#include "protocol/pif_xmodem.h"

#include <string.h>


#define TASK_SIZE		10
#define TIMER_1MS_SIZE	5

#define CHANGE_UART_NONE			0
#define CHANGE_UART_LOG_TO_XMODEM	1
#define CHANGE_UART_XMODEM_TO_LOG	2


PifUart g_uart;
PifTimerManager g_timer_1ms;

static PifTask* s_task;
static PifXmodem s_xmodem;

static int _cmdBasicDownload(int argc, char* argv[]);
static int _cmdBasicPrint(int argc, char* argv[]);
static int _cmdBasicHex(int argc, char* argv[]);
static int _cmdBasicExecute(int argc, char* argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "download", _cmdBasicDownload, "Download Basic Program", NULL },
	{ "print", _cmdBasicPrint, "Print Basic Program", NULL },
	{ "hex", _cmdBasicHex, "Print Hex Code", NULL },
	{ "exec", _cmdBasicExecute, "Run Basic Program", NULL },

	{ NULL, NULL, NULL, NULL }
};

static char s_basic[1024];
static int s_basic_length;
static int s_change_uart = CHANGE_UART_NONE;


static int _basicProcess1(int count, int* p_params);
static int _basicProcess2(int count, int* p_params);

static PifBasicProcess p_process[] = {
		_basicProcess1,
		_basicProcess2,
		NULL,
};


static int _basicProcess1(int count, int* p_params)
{
	pifLog_Printf(LT_INFO, "P1 = %d :", count);
	for (int i = 0; i < count; i++) {
		pifLog_Printf(LT_NONE, " %d", p_params[i]);
	}
	return p_params[0] + 1;
}

static int _basicProcess2(int count, int* p_params)
{
	pifLog_Printf(LT_INFO, "P2 = %d :", count);
	for (int i = 0; i < count; i++) {
		pifLog_Printf(LT_NONE, " %d", p_params[i]);
	}
	return p_params[0] + 2;
}

static int _cmdBasicDownload(int argc, char* argv[])
{
	s_change_uart = CHANGE_UART_LOG_TO_XMODEM;
	pifTask_SetTrigger(s_task);
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdBasicPrint(int argc, char* argv[])
{
	char buf[66], *p_basic;
	int len = s_basic_length;

	pifLog_Printf(LT_INFO, "Basic Length = %d", len);
	if (len > 0) {
		pifLog_Print(LT_NONE, "\n-----------------------------------------------------\n");
		buf[64] = 0;
		p_basic = s_basic;
		while (len) {
			if (len < 64) {
				pifLog_Print(LT_NONE, p_basic);
				p_basic += len;
				len = 0;
			}
			else {
				strncpy(buf, p_basic, 64);
				pifLog_Print(LT_NONE, buf);
				p_basic += 64;
				len -= 64;
			}
			pifTaskManager_YieldMs(10);
		}
		pifLog_Print(LT_NONE, "\n-----------------------------------------------------");
	}
	else {
		pifLog_Print(LT_INFO, "There is no program.");
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdBasicHex(int argc, char* argv[])
{
	char *p_basic;
	int len = s_basic_length, ptr, i;

	pifLog_Printf(LT_INFO, "Basic Length = %d", len);
	if (len > 0) {
		pifLog_Print(LT_NONE, "\n-----------------------------------------------------\n");
		ptr = 0;
		while (len) {
			p_basic = s_basic + ptr;
			if (len < 16) {
				pifLog_Printf(LT_NONE, "%4X:", ptr);
				for (i = 0; i < len; i++) {
					pifLog_Printf(LT_NONE, " %2X", p_basic[i]);
				}
				ptr += len;
				len = 0;
			}
			else {
				pifLog_Printf(LT_NONE, "%4X: %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X\n", ptr,
						p_basic[0], p_basic[1], p_basic[2], p_basic[3], p_basic[4], p_basic[5], p_basic[6], p_basic[7],
						p_basic[8], p_basic[9], p_basic[10], p_basic[11], p_basic[12], p_basic[13], p_basic[14], p_basic[15]);
				ptr += 16;
				len -= 16;
			}
			pifTaskManager_YieldMs(10);
		}
		pifLog_Print(LT_NONE, "\n-----------------------------------------------------");
	}
	else {
		pifLog_Print(LT_INFO, "There is no program.");
	}
	return PIF_LOG_CMD_NO_ERROR;
}

static int _cmdBasicExecute(int argc, char* argv[])
{
	pifBasic_Execute(s_basic, 32);		// 32 :  Count of opcodes processed at one time
	return PIF_LOG_CMD_NO_ERROR;
}

static void _evtXmodemRxReceive(uint8_t code, PifXmodemPacket* p_packet)
{
	char *p_basic;
	int i;

	if (code == ASCII_SOH) {
		p_basic = s_basic + s_basic_length;
		strncpy(p_basic, (char*)p_packet->p_data, 128);
		s_basic_length += 128;
		p_basic[s_basic_length] = 0;
	}
	else if (code == XTS_EOT) {
		p_basic = s_basic + s_basic_length - 128;
		for (i = 127; i >= 0; i--) {
			if (p_basic[i] != 0x1A) break;
			p_basic[i] = 0;
			s_basic_length--;
		}

		s_change_uart = CHANGE_UART_XMODEM_TO_LOG;
		pifTask_SetTrigger(s_task);
	}
}

static uint16_t _taskChangeUart(PifTask* p_task)
{
	(void)p_task;

	switch (s_change_uart) {
	case CHANGE_UART_LOG_TO_XMODEM:
		pifLog_DetachUart();
	    pifXmodem_AttachUart(&s_xmodem, &g_uart);

	    pifXmodem_ReadyReceive(&s_xmodem);
		s_basic_length = 0;
		s_change_uart = CHANGE_UART_NONE;
		break;

	case CHANGE_UART_XMODEM_TO_LOG:
		pifXmodem_DetachUart(&s_xmodem);
	    pifLog_AttachUart(&g_uart);

		s_change_uart = CHANGE_UART_NONE;
		break;
	}
    return 0;
}

static void _evtComplete(PifBasic* p_owner)
{
	char *sym[2] = { "<=", ">" };

	pifLog_Print(LT_INFO, "Complete:");
	pifLog_Printf(LT_NONE, "\n\tProgram Size = %d %s %d", p_owner->_program_size, sym[p_owner->_program_size > PIF_BASIC_PROGRAM], PIF_BASIC_PROGRAM);
	pifLog_Printf(LT_NONE, "\n\tVariable Count = %d %s %d", p_owner->_varable_count, sym[p_owner->_varable_count > PIF_BASIC_VARIABLE], PIF_BASIC_VARIABLE);
	pifLog_Printf(LT_NONE, "\n\tString Count = %d %s %d", p_owner->_string_count, sym[p_owner->_string_count > PIF_BASIC_STRING], PIF_BASIC_STRING);
	pifLog_Printf(LT_NONE, "\n\tStack Count = %d %s %d", p_owner->_stack_count, sym[p_owner->_stack_count > PIF_BASIC_STACK], PIF_BASIC_STACK);
	pifLog_Printf(LT_NONE, "\n\tParsing Time = %ldms", p_owner->_parsing_time);
	pifLog_Printf(LT_NONE, "\n\tProcess Time = %ldms\n", p_owner->_process_time);
}

void appSetup()
{
	PifLed led_l;

	pif_Init(NULL);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;		// 1000us

	if (!pifUart_Init(&g_uart, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&g_uart, TM_PERIOD_MS, 1, "UartLog")) return;					// 1ms
	if (!pifUart_AllocRxBuffer(&g_uart, 64, 100)) return;									// 100%
	if (!pifUart_AllocTxBuffer(&g_uart, 128)) return;
	g_uart.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachUart(&g_uart)) return;
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return;

    if (!pifLed_Init(&led_l, PIF_ID_AUTO, &g_timer_1ms, 2, actLedLState)) return;
    if (!pifLed_AttachSBlink(&led_l, 500)) return;											// 500ms
    pifLed_SBlinkOn(&led_l, 1 << 0);

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return;
    pifXmodem_AttachEvtRxReceive(&s_xmodem, _evtXmodemRxReceive);

	pifBasic_Init(p_process, _evtComplete);

	s_task = pifTaskManager_Add(TM_EXTERNAL_ORDER, 0, _taskChangeUart, NULL, FALSE);
	if (!s_task) return;

	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}