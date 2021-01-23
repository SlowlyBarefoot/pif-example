// Do not remove the include below
#include <MsTimer2.h>
#include <SoftwareSerial.h>

#include "exXmodemSerialRx.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifXmodem.h"


#define PIN_LED_L				13

#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              4
#define XMODEM_COUNT         	1


static SoftwareSerial SwSerial(7, 8);

static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stXmodem *s_pstXmodem = NULL;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _evtXmodemRxReceive(uint8_t ucCode, PIF_stXmodemPacket *pstPacket)
{
	if (ucCode == ASCII_SOH) {
		pifLog_Printf(LT_enInfo, "Code=%u PN=%u", ucCode, (unsigned int)pstPacket->aucPacketNo[0]);
	}
	else {
		pifLog_Printf(LT_enInfo, "Code=%u", ucCode);
	}
}

static void _taskXmodemTest(PIF_stTask *pstTask)
{
	uint8_t txData, rxData;

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

static void _taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void sysTickHook()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200);
	SwSerial.begin(9600);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;

    if (!pifXmodem_Init(s_pstTimer1ms, XMODEM_COUNT)) return;
    s_pstXmodem = pifXmodem_Add(PIF_ID_AUTO, XT_CRC);
    if (!s_pstXmodem) return;
    pifXmodem_AttachComm(s_pstXmodem, s_pstSerial);
    pifXmodem_AttachEvent(s_pstXmodem, NULL, _evtXmodemRxReceive);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodUs(500, pifComm_taskAll, NULL)) return;	// 500us

    if (!pifTask_AddPeriodUs(500, _taskXmodemTest, NULL)) return;	// 500us
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;	// 500ms

    pifXmodem_ReadyReceive(s_pstXmodem);
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
