#include "exProtocolSerialLoopS.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSwitch.h"


#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define SWITCH_COUNT            2

#define PIF_ID_SWITCH			0x100
#define PIF_ID_2_INDEX(id)		((id) - PIF_ID_SWITCH)


static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion21(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x20, PF_enLogPrint_Yes, _fnProtocolQuestion20 },
		{ 0x21, PF_enLogPrint_Yes, _fnProtocolQuestion21 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequests[] = {
		{ 0x30, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_enResponse_Ack | PF_enLogPrint_Yes, _fnProtocolResponse31, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
	PIF_stSwitch *pstPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
	PIF_stSwitchFilter stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED, NULL, 0, },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW, NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_enInfo, "%s: CNT=%u", pcName, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_enInfo, pcName);
	}
}

static void _fnCompareData(PIF_stProtocolPacket *pstPacket, uint8_t ucIndex)
{
	uint16_t i;

	if (pstPacket->usDataCount == s_stProtocolTest[ucIndex].ucDataCount) {
		for (i = 0; i < pstPacket->usDataCount; i++) {
			if (pstPacket->pucData[i] != s_stProtocolTest[ucIndex].ucData[i]) break;
		}
		if (i < pstPacket->usDataCount) {
			pifLog_Printf(LT_enInfo, "Different data");
		}
		else {
			pifLog_Printf(LT_enInfo, "Same data");
		}
	}
	else {
		pifLog_Printf(LT_enError, "Different count: %u != %u", s_stProtocolTest[ucIndex].ucDataCount, pstPacket->usDataCount);
	}
}

static void _fnProtocolQuestion20(PIF_stProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Question20");

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Question20: Error=%d", pif_enError);
	}
}

static void _fnProtocolQuestion21(PIF_stProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Question21");
}

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_enInfo, "Response31: ACK");
}

static void _evtProtocolError(PIF_usId usPifId)
{
	pifLog_Printf(LT_enError, "ProtocolError DC=%d", usPifId);
}

static SWITCH _actPushSwitchAcquire(PIF_usId usPifId)
{
	return digitalRead(s_stProtocolTest[PIF_ID_2_INDEX(usPifId)].ucPinSwitch);
}

static void _evtPushSwitchChange(PIF_usId usPifId, SWITCH swState, void *pvIssuer)
{
	uint8_t index = PIF_ID_2_INDEX(usPifId);

	(void)pvIssuer;

	if (swState) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, &stProtocolRequests[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_enError, "PushSwitchChange(%d): DC=%d E=%d", index, s_pstProtocol->usPifId, pif_enError);
		}
		else {
			pifLog_Printf(LT_enInfo, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_pstProtocol->usPifId, s_stProtocolTest[index].ucDataCount);
			if (s_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_enNone, "\nData:");
				for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
				}
			}
		}
	}
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
    	Serial1.print((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
		rxData = Serial1.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
    }
}

BOOL exSerial1_Setup()
{
	int i;

	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

	Serial1.begin(115200);

    if (!pifSwitch_Init(SWITCH_COUNT)) return FALSE;

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stProtocolTest[i].pstPushSwitch = pifSwitch_Add(PIF_ID_SWITCH + i, 0);
		if (!s_stProtocolTest[i].pstPushSwitch) return FALSE;
		s_stProtocolTest[i].pstPushSwitch->bStateReverse = TRUE;
		pifSwitch_AttachAction(s_stProtocolTest[i].pstPushSwitch, _actPushSwitchAcquire);
		pifSwitch_AttachEvtChange(s_stProtocolTest[i].pstPushSwitch, _evtPushSwitchChange, NULL);
	    if (!pifSwitch_AttachFilter(s_stProtocolTest[i].pstPushSwitch, PIF_SWITCH_FILTER_COUNT, 7, &s_stProtocolTest[i].stPushSwitchFilter)) return FALSE;
    }

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return FALSE;

    s_pstProtocol = pifProtocol_Add(PIF_ID_AUTO, PT_enSmall, stProtocolQuestions);
    if (!s_pstProtocol) return FALSE;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    pifProtocol_AttachEvent(s_pstProtocol, _evtProtocolError);

    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return FALSE;		// 3%
    if (!pifTask_AddRatio(3, _taskProtocolTest, NULL)) return FALSE;		// 3%

    return TRUE;
}
