#include "appMain.h"
#include "exXmodemSerialTx.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifXmodem.h"


#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              3
#define XMODEM_COUNT         	1


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stComm *s_pstSerial = NULL;
static PIF_stXmodem *s_pstXmodem = NULL;


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
//		pifXmodem_SendData(s_pstXmodem, ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		pifXmodem_SendCancel(s_pstXmodem);
		break;

	case ASCII_CAN:
		break;
	}
	pifLog_Printf(LT_enInfo, "C=%u PN=%u L=%u P=%u S=%u", ucCode, ucPacketNo, s_stXmodemTest.usLength, s_stXmodemTest.usPos, s_stXmodemTest.step);
}

void appSetup()
{
    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;
	pifComm_AttachAction(s_pstSerial, actXmodemReceiveData, actXmodemSendData);

    if (!pifXmodem_Init(g_pstTimer1ms, XMODEM_COUNT)) return;
    s_pstXmodem = pifXmodem_Add(PIF_ID_AUTO, XT_enCRC);
	if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvent(s_pstXmodem, _evtXmodemTxReceive, NULL);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodUs(500, pifComm_taskAll, NULL)) return;	// 500us

    if (!pifTask_AddPeriodMs(500, taskLedToggle, NULL)) return;	// 500ms
}
