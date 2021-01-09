// Do not remove the include below
#include "exXmodemSerialTx.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifXmodem.h"


#define PIN_LED_L				13

#define COMM_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              4

#define USE_SERIAL_USB			// Linux or Windows
//#define USE_SERIAL_3			// Other Anduino


static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstSerial = NULL;

static struct {
	uint8_t step;
	uint16_t usTotal;
	uint16_t usPos;
	uint16_t usLength;
	uint8_t aucData[128];
} s_stXmodemTest = { 0, 1000, };


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _evtXmodemTxReceive(uint8_t ucCode, uint8_t ucPacketNo)
{
	uint16_t i;

	switch (ucCode) {
	case 'C':
		s_stXmodemTest.step = 1;
		s_stXmodemTest.usPos = 0;
		s_stXmodemTest.usLength = 128;
		for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
		pifXmodem_SendData(1, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		break;

	case ASCII_ACK:
		if (s_stXmodemTest.step) {
			s_stXmodemTest.usPos += s_stXmodemTest.usLength;
			if (s_stXmodemTest.usTotal > s_stXmodemTest.usPos) {
				ucPacketNo++;
				s_stXmodemTest.usLength = s_stXmodemTest.usTotal - s_stXmodemTest.usPos;
				if (s_stXmodemTest.usLength > 128) s_stXmodemTest.usLength = 128;
				for (i = 0; i < s_stXmodemTest.usLength; i++) s_stXmodemTest.aucData[i] = rand() & 0xFF;
				pifXmodem_SendData(ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
			}
			else {
				pifXmodem_SendEot();
				s_stXmodemTest.step = 0;
			}
		}
		break;

	case ASCII_NAK:
//		pifXmodem_SendData(ucPacketNo, s_stXmodemTest.aucData, s_stXmodemTest.usLength);
		pifXmodem_SendCancel();
		break;

	case ASCII_CAN:
		break;
	}
	pifLog_Printf(LT_enInfo, "C=%u PN=%u L=%u P=%u S=%u", ucCode, ucPacketNo, s_stXmodemTest.usLength, s_stXmodemTest.usPos, s_stXmodemTest.step);
}

static void _taskXmodemTest(PIF_stTask *pstTask)
{
	uint8_t txData, rxData;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
#ifdef USE_SERIAL_USB
    	SerialUSB.print((char)txData);
#endif
#ifdef USE_SERIAL_3
    	Serial3.print((char)txData);
#endif
    }

    while (pifComm_GetRemainSizeOfRxBuffer(s_pstSerial)) {
#ifdef USE_SERIAL_USB
		if (SerialUSB.available()) {
			rxData = SerialUSB.read();
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
#endif
#ifdef USE_SERIAL_3
		if (Serial3.available()) {
			rxData = Serial3.read();
			pifComm_ReceiveData(s_pstSerial, rxData);
		}
		else break;
#endif
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
	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(115200); //Doesn't matter speed
#ifdef USE_SERIAL_USB
	SerialUSB.begin(115200);
#endif
#ifdef USE_SERIAL_3
	Serial3.begin(115200);
#endif

    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) return;

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT);
    if (!s_pstTimer) return;

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) return;

    if (!pifXmodem_Init(s_pstTimer, XT_CRC)) return;
    pifXmodem_AttachComm(s_pstSerial);
    pifXmodem_AttachEvent(_evtXmodemTxReceive, NULL);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodUs(500, pifComm_taskAll, NULL)) return;	// 500us

    if (!pifTask_AddPeriodUs(500, _taskXmodemTest, NULL)) return;	// 500us
    if (!pifTask_AddPeriodMs(500, _taskLedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
