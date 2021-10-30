#include "appMain.h"
#include "exXmodemSerialTx.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_xmodem.h"


PifPulse *g_pstTimer1ms = NULL;

static PifComm *s_pstSerial = NULL;
static PifXmodem *s_pstXmodem = NULL;


static struct {
	uint8_t step;
	uint8_t retry;
	uint16_t usTotal;
	uint16_t usPos;
	uint16_t usLength;
	uint8_t aucData[128];
} s_stXmodemTest = { 0, 0, 1000, };


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
		pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
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
				pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
				s_stXmodemTest.retry = 3;
			}
			else {
				pifXmodem_SendEot(s_pstXmodem);
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
			pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
//			pifXmodem_SendCancel(s_pstXmodem);
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
	PifComm *pstCommLog;
	PifLed *pstLedL;

    pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;		// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
	s_pstSerial->act_receive_data = actXmodemReceiveData;
	s_pstSerial->act_send_data = actXmodemSendData;

    s_pstXmodem = pifXmodem_Create(PIF_ID_AUTO, g_pstTimer1ms, XT_CRC);
	if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvtTxReceive(s_pstXmodem, _evtXmodemTxReceive);
}
