#include "appMain.h"
#include "main.h"

#include "pif_log.h"
#include "pif_xmodem.h"


PifTimerManager *g_pstTimer1ms = NULL;

static PifComm *s_pstCommLog = NULL;
static PifComm *s_pstSerial = NULL;
static PifXmodem *s_pstXmodem = NULL;


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
		pifXmodem_SendData(s_pstXmodem, 1, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		break;

	case ASCII_ACK:
		if (s_stXmodemTest.step) {
			s_stXmodemTest.usPos += s_stXmodemTest.usLength;
			if (s_stXmodemTest.usTotal > s_stXmodemTest.usPos) {
				ucPacketNo++;
				s_stXmodemTest.usLength = s_stXmodemTest.usTotal - s_stXmodemTest.usPos;
				if (s_stXmodemTest.usLength > 128) s_stXmodemTest.usLength = 128;
				for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
				pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
			}
			else {
				pifXmodem_SendEot(s_pstXmodem);
				s_stXmodemTest.step = 0;
			}
		}
		break;

	case ASCII_NAK:
		pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
//		pifXmodem_SendCancel(s_pstXmodem);
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

    g_pstTimer1ms = pifTimerManager_Create(PIF_ID_AUTO, 1000, 2);							// 1000us
    if (!g_pstTimer1ms) return FALSE;

    s_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstCommLog) return FALSE;
	s_pstCommLog->act_send_data = actLogSendData;
    if (!pifComm_AttachTask(s_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

	if (!pifLog_AttachComm(s_pstCommLog)) return FALSE;

    s_pstSerial = pifComm_Create(PIF_ID_AUTO);
	if (!s_pstSerial) return FALSE;
	s_pstSerial->act_receive_data = actSerialReceiveData;
	s_pstSerial->act_send_data = actSerialSendData;
    if (!pifComm_AttachTask(s_pstSerial, TM_PERIOD_MS, 1, TRUE)) return FALSE;		// 1ms

    s_pstXmodem = pifXmodem_Create(PIF_ID_AUTO, g_pstTimer1ms, XT_CRC);
    if (!s_pstXmodem) return FALSE;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvtTxReceive(s_pstXmodem, _evtXmodemTxReceive);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifTimerManager_Count(g_pstTimer1ms));
    return TRUE;
}

void appExit()
{
	pifXmodem_Destroy(&s_pstXmodem);
	pifTimerManager_Destroy(&g_pstTimer1ms);
	pifComm_Destroy(&s_pstSerial);
	pifComm_Destroy(&s_pstCommLog);
    pifLog_Clear();
    pif_Exit();
}
