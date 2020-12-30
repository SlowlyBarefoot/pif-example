// Do not remove the include below
#include <MsTimer2.h>
#include <Wire.h>

#include "exProtocolI2C.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"


#define PIN_LED_L				13

#define COMM_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	8
#define TASK_COUNT              4


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstI2C = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enDefault, _fnProtocolQuestion30 },
		{ 0x31, PF_enDefault, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

static struct {
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ 0, },
		{ 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_enInfo, "%s: CNT=%u", pcName, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (uint16_t i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_enInfo, pcName);
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
}

static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
	s_stProtocolTest[1].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}
}

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", unDeviceCode);
}

static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void receiveEvent(int parameter)
{
	uint8_t rxData;

	(void)parameter;

	int size = pifComm_GetRemainSizeOfRxBuffer(s_pstI2C);
    for (int i = 0; i < size; i++) {
    	if (Wire.available() > 0) {
			rxData = Wire.read();
			pifComm_ReceiveData(s_pstI2C, rxData);
    	}
		else break;
    }
}

static void requestEvent()
{
	uint8_t txData = 0;

	pifComm_SendData(s_pstI2C, &txData);
	Wire.write((char)txData);
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

	Wire.begin(1);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    s_pstI2C = pifComm_Add(unDeviceCode++);
	if (!s_pstI2C) return;

    if (!pifProtocol_Init(s_pstTimer, PROTOCOL_COUNT)) return;
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enSmall, stProtocolQuestions);
    if (!s_pstProtocol) return;
    pifProtocol_AttachComm(s_pstProtocol, s_pstI2C);
    s_pstProtocol->evtError = _evtProtocolError;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodUs(300, pifComm_taskAll, NULL)) return;	// 300us

    if (!pifTask_AddPeriodMs(500, _LedToggle, NULL)) return;		// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
