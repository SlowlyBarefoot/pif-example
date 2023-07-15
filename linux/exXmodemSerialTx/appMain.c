#include "appMain.h"
#include "main.h"

#include "core/pif_log.h"
#include "protocol/pif_xmodem.h"


PifTimerManager g_timer_1ms;

static PifUart s_uart_log;
static PifUart s_serial;
static PifXmodem s_xmodem;


static struct {
	uint8_t step;
	uint16_t usTotal;
	uint16_t usPos;
	uint16_t usLength;
	uint8_t aucData[128];
} s_stXmodemTest = { 0, 1000, };


static void _evtXmodemTxReceive(uint8_t ucCode, uint8_t ucPacketNo)
{
	uint16_t i;

	switch (ucCode) {
	case 'C':
		s_stXmodemTest.step = 1;
		s_stXmodemTest.usPos = 0;
		s_stXmodemTest.usLength = 128;
		for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
		pifXmodem_SendData(&s_xmodem, 1, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		break;

	case ASCII_ACK:
		if (s_stXmodemTest.step) {
			s_stXmodemTest.usPos += s_stXmodemTest.usLength;
			if (s_stXmodemTest.usTotal > s_stXmodemTest.usPos) {
				ucPacketNo++;
				s_stXmodemTest.usLength = s_stXmodemTest.usTotal - s_stXmodemTest.usPos;
				if (s_stXmodemTest.usLength > 128) s_stXmodemTest.usLength = 128;
				for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
				pifXmodem_SendData(&s_xmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
			}
			else {
				pifXmodem_SendEot(&s_xmodem);
				s_stXmodemTest.step = 0;
			}
		}
		break;

	case ASCII_NAK:
		pifXmodem_SendData(&s_xmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
//		pifXmodem_SendCancel(&s_xmodem);
		break;

	case ASCII_CAN:
		break;
	}
	if (s_stXmodemTest.step) {
		pifLog_Printf(LT_INFO, "C=%u PN=%u L=%u P=%u S=%u DT=%2xh", ucCode, ucPacketNo, s_stXmodemTest.usLength,
				s_stXmodemTest.usPos, s_stXmodemTest.step, s_stXmodemTest.aucData[0]);
	}
	else {
		pifLog_Printf(LT_INFO, "C=%u", ucCode);
	}
}

BOOL appInit()
{
    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return FALSE;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return FALSE;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

	if (!pifLog_AttachUart(&s_uart_log)) return FALSE;

	if (!pifUart_Init(&s_serial, PIF_ID_AUTO)) return FALSE;
	s_serial.act_receive_data = actSerialReceiveData;
	s_serial.act_send_data = actSerialSendData;
    if (!pifUart_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return FALSE;
    pifXmodem_AttachUart(&s_xmodem, &s_serial);
    pifXmodem_AttachEvtTxReceive(&s_xmodem, _evtXmodemTxReceive);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
    return TRUE;
}

void appExit()
{
	pifXmodem_Clear(&s_xmodem);
	pifTimerManager_Clear(&g_timer_1ms);
	pifUart_Clear(&s_serial);
	pifUart_Clear(&s_uart_log);
    pifLog_Clear();
    pif_Exit();
}
