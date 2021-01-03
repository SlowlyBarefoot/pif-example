// Do not remove the include below
#include <Wire.h>

#include "exProtocolI2C.h"

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
#define PULSE_ITEM_COUNT    	10
#define SWITCH_COUNT            2
#define TASK_COUNT              5

#define DEVICECODE_SWITCH		10
#define DEVICECODE_2_INDEX(dc)	((dc) - DEVICECODE_SWITCH)


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstI2C = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket);

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

static void _fnProtocolResponse30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_enInfo, "Response31: ACK");
}

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "ProtocolError DC=%d", unDeviceCode);
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

	if (swState) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(s_pstProtocol, &stProtocolRequests[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_enError, "PushSwitchChange(%d): DC=%d E=%d", index, s_pstProtocol->unDeviceCode, pif_enError);
		}
		else {
			pifLog_Printf(LT_enInfo, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_pstProtocol->unDeviceCode, s_stProtocolTest[index].ucDataCount);
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
	static int stepRead = 0;

	(void)pstTask;

	if (pifComm_GetFillSizeOfTxBuffer(s_pstI2C)) {
		Wire.beginTransmission(1);
		for (int i = 0; i < PIF_COMM_RX_BUFFER_SIZE; i++) {
			if (pifComm_SendData(s_pstI2C, &txData)) {
				Wire.write((char)txData);
				if (txData == ASCII_ETX) {
					stepRead = 1;
				}
			}
			else break;
		}
		Wire.endTransmission();
	}

    switch (stepRead) {
    case 1:
    	if (pifComm_GetRemainSizeOfRxBuffer(s_pstI2C)) {
			Wire.requestFrom(1, 1);
			stepRead = 2;
    	}
        break;

    case 2:
    	if (Wire.available() > 0) {
			rxData = Wire.read();
			if (rxData > 0) {
				pifComm_ReceiveData(s_pstI2C, rxData);
				switch (rxData) {
				case ASCII_ETX:
				case ASCII_ACK:
				case ASCII_NAK:
					stepRead = 0;
					break;

				default:
					stepRead = 1;
					break;
				}
			}
    	}
    	break;
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

	Wire.begin();

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
		pifSwitch_AttachAction(s_stProtocolTest[i].pstPushSwitch, _actPushSwitchAcquire);
		pifSwitch_AttachEvent(s_stProtocolTest[i].pstPushSwitch, _evtPushSwitchChange);
	    if (!pifSwitch_AttachFilter(s_stProtocolTest[i].pstPushSwitch, PIF_SWITCH_FILTER_COUNT, 7, &s_stProtocolTest[i].stPushSwitchFilter)) return;
    }

    s_pstI2C = pifComm_Add(unDeviceCode++);
	if (!s_pstI2C) return;

    if (!pifProtocol_Init(s_pstTimer, PROTOCOL_COUNT)) return;
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enSmall, NULL);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstI2C);
    pifProtocol_AttachEvent(s_pstProtocol, _evtProtocolError);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;		// 3%
    if (!pifTask_AddPeriodUs(300, pifComm_taskAll, NULL)) return;	// 300us

    if (!pifTask_AddPeriodUs(300, _taskProtocolTest, NULL)) return;	// 300us
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
