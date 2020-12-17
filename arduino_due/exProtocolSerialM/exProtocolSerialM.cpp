// Do not remove the include below
#include "exProtocolSerialM.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSwitch.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_LED_GREEN			27
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31
#define PIN_PUSH_SWITCH_3		33

#define COMM_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SWITCH_COUNT            3
#define TASK_COUNT              5

#define DEVICECODE_SWITCH		10
#define DEVICECODE_2_INDEX(dc)	((dc) - DEVICECODE_SWITCH)


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion21(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion22(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x20, PF_enLogPrint_Yes, _fnProtocolQuestion20 },
		{ 0x21, PF_enLogPrint_Yes, _fnProtocolQuestion21 },
		{ 0x22, PF_enLogPrint_Yes, _fnProtocolQuestion22 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequests[] = {
		{ 0x30, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_enResponse_Ack | PF_enLogPrint_Yes, _fnProtocolResponse31, 3, 300 },
		{ 0x32, PF_enResponse_No | PF_enLogPrint_Yes, NULL, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
	PIF_stSwitch *pstPushSwitch;
	PIF_stSwitchFilter stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED, NULL },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW, NULL },
		{ PIN_PUSH_SWITCH_3, PIN_LED_GREEN, NULL }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, char *pcName)
{
	pifLog_Printf(LT_enInfo, "%s: PID=%d DC=%u", pcName, pstPacket->ucPacketId, pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
	}
}

static void _fnProtocolQuestion20(PIF_stProtocolPacket *pstPacket)
{
	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Question20: PID=%d Error=%d", pstPacket->ucPacketId, pif_enError);
	}
	else {
		_fnProtocolPrint(pstPacket, "Question20");
	}
}

static void _fnProtocolQuestion21(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question21");
}

static void _fnProtocolQuestion22(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question22");
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

static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static SWITCH _actPushSwitchAcquire(PIF_unDeviceCode unDeviceCode)
{
	return digitalRead(s_stProtocolTest[DEVICECODE_2_INDEX(unDeviceCode)].ucPinSwitch);
}

static void _evtPushSwitchChange(PIF_unDeviceCode unDeviceCode, SWITCH swState)
{
	uint8_t index = DEVICECODE_2_INDEX(unDeviceCode);
	uint8_t ucData[8];

	if (swState) {
		for (int i = 0; i < 8; i++) ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, &stProtocolRequests[index], ucData, 8)) {
			pifLog_Printf(LT_enError, "PushSwitchChange: DC=%d IDX=%d E=%d", s_pstProtocol->unDeviceCode, index, pif_enError);
		}
		else {
			pifLog_Printf(LT_enInfo, "PushSwitchChange: DC=%d IDX=%d", s_pstProtocol->unDeviceCode, index);
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < 8; i++) {
				pifLog_Printf(LT_enNone, " %u", ucData[i]);
			}
		}
	}
}

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "ProtocolError DC=%d", unDeviceCode);
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
    	SerialUSB.print((char)txData);
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
		rxData = SerialUSB.read();
		if (rxData >= 0) {
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
    }
}

static void _taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	PIF_unDeviceCode unDeviceCode = 1;
	int i;

	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_3, INPUT_PULLUP);

	Serial.begin(115200);
	SerialUSB.begin(115200);

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    for (i = 0; i < SWITCH_COUNT; i++) {
    	s_stProtocolTest[i].pstPushSwitch = pifSwitch_Add(DEVICECODE_SWITCH + i, 0);
		if (!s_stProtocolTest[i].pstPushSwitch) return;
		s_stProtocolTest[i].pstPushSwitch->bStateReverse = TRUE;
		s_stProtocolTest[i].pstPushSwitch->actAcquire = _actPushSwitchAcquire;
		s_stProtocolTest[i].pstPushSwitch->evtChange = _evtPushSwitchChange;
	    if (!pifSwitch_AttachFilter(s_stProtocolTest[i].pstPushSwitch, PIF_SWITCH_FILTER_COUNT, 7, &s_stProtocolTest[i].stPushSwitchFilter)) return;
    }

    s_pstSerial = pifComm_Add(unDeviceCode++);
	if (!s_pstSerial) return;

    if (!pifProtocol_Init(s_pstTimer, PROTOCOL_COUNT)) return;
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enMedium, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evtError = _evtProtocolError;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;		// 3%
    if (!pifTask_AddRatio(3, pifComm_taskAll, NULL)) return;		// 3%

    if (!pifTask_AddRatio(3, _taskProtocolTest, NULL)) return;		// 3%
    if (!pifTask_AddPeriod(500, _taskLedToggle, NULL)) return;		// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
