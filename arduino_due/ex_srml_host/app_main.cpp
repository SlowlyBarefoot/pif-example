/**
 * @file app_main.cpp
 * @brief Arduino DUE examples for Simple Receipt Markup Langeage
 * @details I used the ex_uart_fc_device example to test this example. I don't have a receipt printer so I haven't tested it.
 *
 *  Conversion
 *   Command		Value		Explanation
 * ------------------------------------------------------
 * %[<|=|>][n]c		String		Complny	name
 * %[<|=|>][n]a		String		Address
 * %[<|=|>][n]p		String		Phone
 * %[<|=|>][n]d		String		Date/Time
 * %[<|=|>][n]m		String		Terminal number
 * %[<|=|>][n]n		String		Partner name
 * %[<|=|>][n]i		String		Product name
 * %[<|=|>][n.m]r	Real		Price
 * %[<|=|>][n]q		Integer		Qty
 * %[<|=|>][n.m]u	Real		Amount
 * %[<|=|>][n.m]s	Real		Subtotal
 * %[<|=|>][n.m]x	Real		Total tax
 * %[<|=|>][n.m]t	Real		Total
 * %[<|=|>][n.m]h	Real		Cash
 *
 * Repeat
 * Command		Explanation
 * ----------------------------------------------------------------------------------------------------------------------
 *   [P   		If this character exists from the first column, it is repeated as many times as the number of products.
 *   [p			If this character is present from the first column, it is repeated as many times as the number of products,
 *   			but is output only if qty is 0 or more.
 *
 *   if
 * Command		Explanation
 * ----------------------------------------------------------------------------------------------------------------------
 *   ?a			If the thousands separator is not ',', the company name is printed at the bottom of the receipt.
 */

#include "linker.h"

#include "markup/pif_srml.h"

#include <stdlib.h>


#define MAX_PRODUCT_COUNT     20


typedef struct
{
    char company[30];
    char address[34];
    char phone[16];
    char date[20];
    char terminal[10];
    char partner[20];
    int product_count;
    char product_item[MAX_PRODUCT_COUNT][20];
    double product_price[MAX_PRODUCT_COUNT];
    long product_qty[MAX_PRODUCT_COUNT];
    double product_amount[MAX_PRODUCT_COUNT];
    double subtotal;
    double tax;
    double total;
    double cash;
} SrmlParam;


PifLed g_led_l;
PifTimerManager g_timer_1ms;
PifUart g_uart_printer;

static PifSrml s_srml;

static SrmlParam s_param;

const char format_1[] = " \n \n\
%=32c\n\
%=32a\n\
Tel: %p\n\
Date: %d\n\
Termal: %m\n\
Partner: %n\n\n\
ITEM NAME          QTY    AMOUNT\n\
#32-\n\
[P%18i %>3q %>9u\n\
#32-\n\
Subtotal %>23s\n\
Total tax %>22x\n\
Total %>26t\n\
#32-\n\
Cash %>27h\n\
?a#32-\n\
?a%=32c\n";

const char format_2[] = " \n \n\
%=32c\n\
%=32a\n\
Tel: %p\n\
Date: %d\n\
Termal: %m\n\
Partner: %n\n\n\
ITEM NAME          QTY    AMOUNT\n\
#32-\n\
[p%18i %>3q %9.2u\n\
#32-\n\
Subtotal %23.2s\n\
Total tax %22.2x\n\
Total %26.2t\n\
#32-\n\
Cash %27.2h\n\
?a#32-\n\
?a%=32c\n";


static PifSrmlType _process_data(PifSrml *p_owner, char command, void **p_data, int loop_idx)
{
	switch (command) {
	case 'c': *p_data = s_param.company; return SRMLT_STRING;
	case 'a': *p_data = s_param.address; return SRMLT_STRING;
	case 'p': *p_data = s_param.phone; return SRMLT_STRING;
	case 'd': *p_data = s_param.date; return SRMLT_STRING;
	case 'm': *p_data = s_param.terminal; return SRMLT_STRING;
	case 'n': *p_data = s_param.partner; return SRMLT_STRING;
	case 'i': *p_data = s_param.product_item[loop_idx]; return SRMLT_STRING;
	case 'r': *p_data = &s_param.product_price[loop_idx]; return SRMLT_REAL;
	case 'q': *p_data = &s_param.product_qty[loop_idx]; return SRMLT_INT;
	case 'u': *p_data = &s_param.product_amount[loop_idx]; return SRMLT_REAL;
	case 's': *p_data = &s_param.subtotal; return SRMLT_REAL;
	case 'x': *p_data = &s_param.tax; return SRMLT_REAL;
	case 't': *p_data = &s_param.total; return SRMLT_REAL;
	case 'h': *p_data = &s_param.cash; return SRMLT_REAL;

	default: *p_data = NULL; return SRMLT_NONE;
	}
}

static BOOL _process_loop(PifSrml *p_owner, char command)
{
    switch (command) {
    case 'p':
        while (!s_param.product_qty[p_owner->loop_idx]) p_owner->loop_idx++;
        break;

    case 'P':
        break;
    }
	return p_owner->loop_idx >= s_param.product_count;
}

static BOOL _process_if(PifSrml *p_owner, char command)
{
	switch (command) {
	case 'a':
	    return p_owner->ch_thousand != ',';

	default:
		return TRUE;
	}
}

static void _line_print(char *message, int length)
{
	int pos = 0, len;

	while (1) {
		len = pifUart_SendTxData(&g_uart_printer, (uint8_t *)message + pos, length - pos);
		if (pos + len < length) pos += len; else break;
		pifTaskManager_YieldMs(1);
	}
	pifTaskManager_YieldMs(5);
}

static void _calculate_data()
{
    int i;

    s_param.subtotal = 0;
    for (i = 0; i < s_param.product_count; i++) {
    	s_param.product_amount[i] = s_param.product_qty[i] * s_param.product_price[i];
    	s_param.subtotal += s_param.product_amount[i];
    }

    s_param.tax = s_param.subtotal / 10;
    s_param.total = s_param.subtotal + s_param.tax;
    s_param.cash = s_param.total;
}

static void _evtUartTxFlowState(void *p_client, SWITCH state)
{
	(void)p_client;

	pifLog_Printf(LT_INFO, "Tx Flow State=%d", state);
}

static void _evtLogControlChar(char ch)
{
	pifLog_Printf(LT_INFO, "Contorl Char = %x\n", ch);
}

static int _CmdFlowControl(int argc, char *argv[])
{
	const char *name[] = { "None", "", "Software", "Hardware", "Software", "Hardware" };

	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Flow Control=%s\n", name[g_uart_printer._flow_control]);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "sw")) {
			pifUart_SetFlowControl(&g_uart_printer, UFC_HOST_SOFTWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "hw")) {
			pifUart_SetFlowControl(&g_uart_printer, UFC_HOST_HARDWARE, _evtUartTxFlowState);
		}
		else if (!strcmp(argv[0], "no")) {
			pifUart_ResetFlowControl(&g_uart_printer);
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdDecimal(int argc, char *argv[])
{
	if (argc == 0) {
		pifLog_Printf(LT_NONE, "  Decimal=%c\n", s_srml.ch_decimal);
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "p")) {
		    s_srml.ch_decimal = '.';
		}
		else if (!strcmp(argv[0], "c")) {
		    s_srml.ch_decimal = ',';
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdThousand(int argc, char *argv[])
{
	if (argc == 0) {
		if (s_srml.ch_thousand)	pifLog_Printf(LT_NONE, "  Thousand=%c\n", s_srml.ch_thousand);
		else pifLog_Print(LT_NONE, "  Thousand=none");
		return PIF_LOG_CMD_NO_ERROR;
	}
	else if (argc > 0) {
		if (!strcmp(argv[0], "p")) {
		    s_srml.ch_thousand = '.';
		}
		else if (!strcmp(argv[0], "c")) {
		    s_srml.ch_thousand = ',';
		}
		else if (!strcmp(argv[0], "s")) {
		    s_srml.ch_thousand = ' ';
		}
		else if (!strcmp(argv[0], "n")) {
		    s_srml.ch_thousand = 0;
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static int _CmdPrinting(int argc, char *argv[])
{
    int i;

	if (argc > 0) {
		if (!strcmp(argv[0], "1")) {
		    strcpy(s_param.terminal, "1234567");
		    strcpy(s_param.partner, "John");

		    s_param.product_count = 4;
		    strcpy(s_param.product_item[0], "Apple");
		    strcpy(s_param.product_item[1], "Melon");
		    strcpy(s_param.product_item[2], "Banana");
		    strcpy(s_param.product_item[3], "Lemon");

		    for (i = 0; i < s_param.product_count; i++) {
		    	s_param.product_price[i] = rand() % 50 * 1000;
		    	s_param.product_qty[i] = rand() % 5;
		    }

		    _calculate_data();

		    pifSrml_Parsing(&s_srml, format_1);
		}
		else if (!strcmp(argv[0], "2")) {
		    strcpy(s_param.terminal, "987654");
		    strcpy(s_param.partner, "Sella");

		    s_param.product_count = 6;
		    strcpy(s_param.product_item[0], "Americano");
		    strcpy(s_param.product_item[1], "Latte");
		    strcpy(s_param.product_item[2], "Espresso");
		    strcpy(s_param.product_item[3], "Cappuccino");
		    strcpy(s_param.product_item[4], "Cafe Ore");
		    strcpy(s_param.product_item[5], "Vienna Coffee");

		    for (i = 0; i < s_param.product_count; i++) {
		    	s_param.product_price[i] = rand() % 5000 / 100.0;
		    	s_param.product_qty[i] = rand() % 5;
		    }

		    _calculate_data();

		    pifSrml_Parsing(&s_srml, format_2);
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

const PifLogCmdEntry c_cmd_table[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "fc", _CmdFlowControl, "Set and print flow control", NULL },
	{ "decimal", _CmdDecimal, "Change decimal character", NULL },
	{ "thousand", _CmdThousand, "Change thousand character", NULL },
	{ "print", _CmdPrinting, "Printing a receipt", NULL },

	{ NULL, NULL, NULL, NULL }
};

BOOL appSetup()
{
	int line;

	if (!pifLog_UseCommand(32, c_cmd_table, "\nDebug> ")) { line = __LINE__; goto fail; }	// 32bytes
	pifLog_AttachEvent(_evtLogControlChar);

	g_uart_printer._p_tx_task->pause = FALSE;

	if (!pifLed_AttachSBlink(&g_led_l, 500)) { line = __LINE__; goto fail; }				// 500ms
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

    strcpy(s_param.company, "Store-Lan");
    strcpy(s_param.address, "Hoan Kiem Dist, Hanoi, Vietnam");
    strcpy(s_param.phone, "(024) 1234-5678");
    strcpy(s_param.date, "2023-08-17 11:20:45");

	if (!pifSrml_Init(&s_srml, _process_data, _process_loop, _process_if, _line_print)) { line = __LINE__; goto fail; }
	return TRUE;

fail:
	pifLog_Printf(LT_INFO, "Setup failed. %d\n", line);
	return FALSE;
}
