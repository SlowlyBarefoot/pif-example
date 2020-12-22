// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exProtocolSerialS.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"


#define PIN_LED_L				13

#define COMM_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	8
#define TASK_COUNT              4


static SoftwareSerial SwSerial(10, 11);

static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enDefault, _fnProtocolQuestion30 },
		{ 0x31, PF_enDefault, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enResponse_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_Ack, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	PIF_stPulseItem *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	pifLog_Printf(LT_enInfo, "%s: CNT=%u", pcName, pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (uint16_t i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
	}
}

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question30");
	s_stProtocolTest[0].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Question30: Error=%d", pif_enError);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
	s_stProtocolTest[1].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response20");
}

static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_enInfo, "Response21: ACK");
}

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", unDeviceCode);
}

static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _evtDelay(void *pvIssuer)
{
	if (!pvIssuer) {
		pif_enError = E_enInvalidParam;
		return;
	}

	const PIF_stProtocolRequest *pstOwner = (PIF_stProtocolRequest *)pvIssuer;
	int index = pstOwner->ucCommand & 0x0F;

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_enError, "Delay(%u): DC=%d E=%d", index, s_pstProtocol->unDeviceCode, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "Delay(%u): DC=%d CNT=%u", index, s_pstProtocol->unDeviceCode, s_stProtocolTest[index].ucDataCount);
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
	}
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	uint8_t rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
    	SwSerial.write((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
    	if (SwSerial.available()) {
			rxData = SwSerial.read();
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
    }
}

static void _LedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void sysTickHook()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer);
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unDeviceCode unDeviceCode = 1;

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed
	SwSerial.begin(28800);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    s_pstSerial = pifComm_Add(unDeviceCode++);
	if (!s_pstSerial) return;

    if (!pifProtocol_Init(s_pstTimer, PROTOCOL_COUNT)) return;
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enSmall, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evtError = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(s_pstTimer, PT_enOnce);
		if (!s_stProtocolTest[i].pstDelay) return;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddRatio(3, pifComm_taskAll, NULL)) return;		// 3%

    if (!pifTask_AddRatio(3, _taskProtocolTest, NULL)) return;		// 3%
    if (!pifTask_AddPeriod(500, _LedToggle, NULL)) return;			// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
