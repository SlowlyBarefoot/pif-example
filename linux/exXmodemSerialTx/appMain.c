#include "appMain.h"

#include "protocol/pif_xmodem.h"


PifUart s_serial;
PifTimerManager g_timer_1ms;

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
    if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return FALSE;
    pifXmodem_AttachUart(&s_xmodem, &s_serial);
    pifXmodem_AttachEvtTxReceive(&s_xmodem, _evtXmodemTxReceive);
    return TRUE;
}

void appExit()
{
	pifXmodem_Clear(&s_xmodem);
}
