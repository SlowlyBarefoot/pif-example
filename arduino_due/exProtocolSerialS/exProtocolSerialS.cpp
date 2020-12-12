// Do not remove the include below
#include "exProtocolSerialS.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"
#include "pifSwitch.h"


#define PIN_LED_L				13
#define PIN_LED_RED				23
#define PIN_LED_YELLOW			25
#define PIN_PUSH_SWITCH_1		29
#define PIN_PUSH_SWITCH_2		31

#define COMM_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SWITCH_COUNT            2
#define TASK_COUNT              5

#define DEVICECODE_SWITCH		10
#define DEVICECODE_2_INDEX(dc)	((dc) - DEVICECODE_SWITCH)


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolFinish20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolFinish21(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolFinish30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolFinish31(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolResponse stProtocolResponseTable[] = {
		{ 0x20, PF_enType_Response | PF_enLogPrint_Yes, _fnProtocolFinish20 },
		{ 0x21, PF_enType_Response | PF_enLogPrint_Yes, _fnProtocolFinish21 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x30, PF_enType_Request | PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolFinish30, 3, 300 },
		{ 0x31, PF_enType_Request | PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolFinish31, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	uint8_t ucPinSwitch;
	uint8_t ucPinLed;
	PIF_stSwitch *pstPushSwitch;
	PIF_stSwitchFilter stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT] = {
		{ PIN_PUSH_SWITCH_1, PIN_LED_RED, NULL },
		{ PIN_PUSH_SWITCH_2, PIN_LED_YELLOW, NULL }
};


static void _fnProtocolFinish20(PIF_stProtocolPacket *pstPacket)
{
	if (!pifProtocol_MakeResponse(s_pstProtocol, pstPacket->ucCommand, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "ProtocolFinish20: Error=%d", pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "ProtocolFinish20: DC=%u", pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
}

static void _fnProtocolFinish21(PIF_stProtocolPacket *pstPacket)
{
	if (!pifProtocol_MakeResponse(s_pstProtocol, pstPacket->ucCommand, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "ProtocolFinish21: Error=%d", pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "ProtocolFinish21: DC=%u", pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
}

static void _fnProtocolFinish30(PIF_stProtocolPacket *pstPacket)
{
	pifLog_Printf(LT_enInfo, "ProtocolFinish30: DC=%u", pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
	}
}

static void _fnProtocolFinish31(PIF_stProtocolPacket *pstPacket)
{
	pifLog_Printf(LT_enInfo, "ProtocolFinish31: DC=%u", pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
	}
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
	uint8_t ucData[4];

	if (swState) {
		for (int i = 0; i < 4; i++) ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, stProtocolRequestTable[index].ucCommand, ucData, 4)) {
			pifLog_Printf(LT_enError, "PushSwitchChange: DC=%d IDX=%d E=%d", s_pstProtocol->unDeviceCode, index, pif_enError);
		}
		else {
			pifLog_Printf(LT_enInfo, "PushSwitchChange: DC=%d IDX=%d", s_pstProtocol->unDeviceCode, index);
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < 4; i++) {
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

    if (pifComm_SendData(s_pstSerial, &txData)) {
    	SerialUSB.print((char)txData);
    }

	rxData = SerialUSB.read();
    if (rxData >= 0) {
    	pifComm_ReceiveData(s_pstSerial, rxData);
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
	pinMode(PIN_PUSH_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_PUSH_SWITCH_2, INPUT_PULLUP);

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
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enSimple, stProtocolRequestTable, stProtocolResponseTable);
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
