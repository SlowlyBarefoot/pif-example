#include "appMain.h"
#include "exXmodemSerialTx.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "protocol/pif_xmodem.h"


PifTimerManager g_timer_1ms;

static PifComm s_serial;
static PifXmodem s_xmodem;


static struct {
	uint8_t step;
	uint8_t retry;
	uint16_t usTotal;
	uint16_t usPos;
	uint16_t usLength;
	uint8_t aucData[128];
} s_stXmodemTest = { 0, 0, 1000, 0, 0, { 0, } };


static void _evtXmodemTxReceive(uint8_t ucCode, uint8_t ucPacketNo)
{
	uint16_t i;

	switch (ucCode) {
	case 'C':
		s_stXmodemTest.step = 1;
		s_stXmodemTest.usPos = 0;
		ucPacketNo++;
		s_stXmodemTest.usLength = 128;
		for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
		pifXmodem_SendData(&s_xmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		s_stXmodemTest.retry = 3;
		break;

	case ASCII_ACK:
		switch (s_stXmodemTest.step) {
		case 1:
			s_stXmodemTest.usPos += s_stXmodemTest.usLength;
			if (s_stXmodemTest.usTotal > s_stXmodemTest.usPos) {
				ucPacketNo++;
				s_stXmodemTest.usLength = s_stXmodemTest.usTotal - s_stXmodemTest.usPos;
				if (s_stXmodemTest.usLength > 128) s_stXmodemTest.usLength = 128;
				for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
				pifXmodem_SendData(&s_xmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
				s_stXmodemTest.retry = 3;
			}
			else {
				pifXmodem_SendEot(&s_xmodem);
				s_stXmodemTest.step = 2;
			}
			break;

		case 2:
			s_stXmodemTest.step = 0;
			break;
		}
		break;

	case ASCII_NAK:
		if (s_stXmodemTest.retry) {
			s_stXmodemTest.retry--;
			pifXmodem_SendData(&s_xmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
//			pifXmodem_SendCancel(&s_xmodem);
		}
		else {
			pifLog_Printf(LT_ERROR, "Send failed");
			s_stXmodemTest.step = 0;
		}
		break;

	case ASCII_CAN:
		break;
	}
	switch (s_stXmodemTest.step) {
	case 0:
		pifLog_Printf(LT_INFO, "C=%u", ucCode);
		break;

	case 1:
		pifLog_Printf(LT_INFO, "C=%u PN=%u L=%u P=%u S=%u DT=%2xh", ucCode, ucPacketNo, s_stXmodemTest.usLength,
				s_stXmodemTest.usPos, s_stXmodemTest.step, s_stXmodemTest.aucData[0]);
		break;

	case 2:
		pifLog_Printf(LT_INFO, "C=%u EOT", ucCode);
		break;
	}
}

void appSetup()
{
	static PifComm s_comm_log;
	static PifLed s_led_l;

    pif_Init(NULL);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;	// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;						// 500ms

	if (!pifComm_Init(&s_serial, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
    s_serial.act_receive_data = actXmodemReceiveData;
    s_serial.act_send_data = actXmodemSendData;

	if (!pifXmodem_Init(&s_xmodem, PIF_ID_AUTO, &g_timer_1ms, XT_CRC)) return;
    pifXmodem_AttachComm(&s_xmodem, &s_serial);
    pifXmodem_AttachEvtTxReceive(&s_xmodem, _evtXmodemTxReceive);

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
