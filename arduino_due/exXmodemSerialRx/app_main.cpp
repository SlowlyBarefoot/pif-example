#include "app_port.h"

#include "protocol/pif_xmodem.h"


#define XMODEM_DATA_SIZE		128
#define TEXT_CHUNK_SIZE			64		// Kept well under the 256-byte log transmit buffer


PifLed g_led_l;
PifSensorSwitch g_push_switch;
PifUart g_serial;
PifTimerManager g_timer_1ms;

static uint32_t s_rx_size;		// Text bytes received so far in this transfer


static void _flushText(char *p_text, int *p_pos)
{
	p_text[*p_pos] = 0;
	pifLog_Print(LT_NONE, p_text);
	*p_pos = 0;

	// The log transmit buffer cannot hold a whole packet if the text is long, so each chunk is sent
	// before the next. The ACK waits meanwhile, which also holds the sender back until it is out.
	pifLog_SendAndExit();
}

static void _printText(PifXmodemPacket *pstPacket)
{
	char text[TEXT_CHUNK_SIZE + 1];
	uint8_t ch;
	int i, pos = 0;

	if (!s_rx_size) pifLog_Print(LT_INFO, "Text:\n");

	for (i = 0; i < XMODEM_DATA_SIZE; i++) {
		ch = pstPacket->p_data[i];

		// The last packet is padded with 1Ah (Ctrl-Z), which also marks the end of a text file.
		if (ch == ASCII_SUB) break;

		// Line breaks and tabs are kept. Other control characters would upset the terminal.
		// Bytes from 80h up are kept, so that UTF-8 text shows as it is.
		if ((ch < 0x20 && ch != '\r' && ch != '\n' && ch != '\t') || ch == 0x7F) ch = '.';

		text[pos++] = ch;
		s_rx_size++;
		if (pos == TEXT_CHUNK_SIZE) _flushText(text, &pos);
	}
	if (pos) _flushText(text, &pos);
}

static void _evtXmodemRxReceive(uint8_t ucCode, PifXmodemPacket *pstPacket)
{
	switch (ucCode) {
	case ASCII_SOH:
		_printText(pstPacket);
		break;

	case ASCII_EOT:
		pifLog_Printf(LT_INFO, "Finished: %lu bytes", (unsigned long)s_rx_size);
		s_rx_size = 0;
		break;

	case ASCII_CAN:
		pifLog_Printf(LT_INFO, "Cancelled: %lu bytes", (unsigned long)s_rx_size);
		s_rx_size = 0;
		break;

	default:
		pifLog_Printf(LT_INFO, "Code=%u", ucCode);
		break;
	}
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	(void)p_owner;
	(void)p_value;
	(void)p_issuer;

	if (state) {
		s_rx_size = 0;
	    if (pifXmodem_ReadyReceive((PifXmodem*)p_issuer)) {
	    	pifLog_Print(LT_INFO, "Ready to receive");
	    }
	}
}

BOOL appSetup()
{
	PifTask* p_task;
	static PifXmodem s_xmodem;

    p_task = pifSensorSwitch_AttachTaskAcquire(&g_push_switch, PIF_ID_AUTO, TM_PERIOD, 10000, TRUE);	// 10ms
    if (!p_task) return FALSE;
    p_task->name = "PushSw";
	pifSensor_AttachEvtChange(&g_push_switch.parent, _evtPushSwitchChange, &s_xmodem);

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, XT_CRC)) return FALSE;
    s_xmodem._p_task->name = "Xmodem";
    pifXmodem_AttachTransport(&s_xmodem, &g_serial.parent);
    pifXmodem_AttachEvtRx(&s_xmodem, _evtXmodemRxReceive, NULL);

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;														// 500ms
    g_led_l._p_task->name = "LedL";
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exXmodemSerialRx         ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
    return TRUE;
}
