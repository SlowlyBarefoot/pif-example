#include "appMain.h"
#include "exProtocolSerialM.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_noise_filter_bit.h"
#include "pif_protocol.h"
#include "pif_sensor_switch.h"


PifTimerManager g_timer_1ms;

static PifComm s_serial;
static PifProtocol s_protocol;

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket);
static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions[] = {
		{ 0x20, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolQuestion20 },
		{ 0x21, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolQuestion21 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequests[] = {
		{ 0x30, PF_RESPONSE_YES | PF_LOG_PRINT_YES, _fnProtocolResponse30, 3, 300 },
		{ 0x31, PF_RESPONSE_NO | PF_LOG_PRINT_YES, _fnProtocolResponse31, 3, 300 },
		{ 0, PF_DEFAULT, NULL, 0, 0 }
};

static struct {
	PifSensorSwitch stPushSwitch;
	uint8_t ucDataCount;
	uint8_t ucData[8];
	PifNoiseFilterBit stPushSwitchFilter;
} s_stProtocolTest[SWITCH_COUNT];


static void _fnProtocolPrint(PifProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_INFO, "%s: PID=%d CNT=%u", pcName, pstPacket->packet_id, pstPacket->data_count);
		if (pstPacket->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (uint16_t i = 0; i < pstPacket->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", pstPacket->p_data[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_INFO, pcName);
	}
}

static void _fnCompareData(PifProtocolPacket *pstPacket, uint8_t ucIndex)
{
	uint16_t i;

	if (pstPacket->data_count == s_stProtocolTest[ucIndex].ucDataCount) {
		for (i = 0; i < pstPacket->data_count; i++) {
			if (pstPacket->p_data[i] != s_stProtocolTest[ucIndex].ucData[i]) break;
		}
		if (i < pstPacket->data_count) {
			pifLog_Printf(LT_INFO, "Different data");
		}
		else {
			pifLog_Printf(LT_INFO, "Same data");
		}
	}
	else {
		pifLog_Printf(LT_ERROR, "Different count: %u != %u", s_stProtocolTest[ucIndex].ucDataCount, pstPacket->data_count);
	}
}

static void _fnProtocolQuestion20(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Question20");

	if (!pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Question20: PID=%d Error=%d", pstPacket->packet_id, pif_error);
	}
}

static void _fnProtocolQuestion21(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Question21");
}

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_INFO, "Response31: ACK");
}

static void _evtProtocolError(PifId usPifId)
{
	pifLog_Printf(LT_ERROR, "ProtocolError DC=%d", usPifId);
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, void* p_issuer)
{
	uint8_t index = p_owner->_id - PIF_ID_SWITCH;

	(void)p_value;
	(void)p_issuer;

	if (state) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(&s_protocol, &stProtocolRequests[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_ERROR, "PushSwitchChange(%d): DC=%d E=%d", index, s_protocol._id, pif_error);
		}
		else {
			pifLog_Printf(LT_INFO, "PushSwitchChange(%d): DC=%d CNT=%u", index, s_protocol._id, s_stProtocolTest[index].ucDataCount);
			if (s_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_NONE, "\nData:");
				for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_NONE, " %u", s_stProtocolTest[index].ucData[i]);
				}
			}
		}
	}
}

void appSetup()
{
	int i;
	static PifComm s_comm_log;
	static PifLed s_led_l;

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 3)) return;											// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;											// 1ms
	s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;																// 500ms

    for (i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifNoiseFilterBit_Init(&s_stProtocolTest[i].stPushSwitchFilter, 7)) return;

	    if (!pifSensorSwitch_Init(&s_stProtocolTest[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire, NULL)) return;
	    if (!pifSensorSwitch_AttachTaskAcquire(&s_stProtocolTest[i].stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return;	// 10ms
		pifSensor_AttachEvtChange(&s_stProtocolTest[i].stPushSwitch.parent, _evtPushSwitchChange);
		s_stProtocolTest[i].stPushSwitch.p_filter = &s_stProtocolTest[i].stPushSwitchFilter.parent;
    	s_stProtocolTest[i].ucDataCount = 0;
    }

	if (!pifComm_Init(&s_serial, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_serial, TM_PERIOD_MS, 1, TRUE)) return;												// 1ms
    s_serial.act_receive_data = actSerialReceiveData;
    s_serial.act_send_data = actSerialSendData;

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_MEDIUM, stProtocolQuestions)) return;
    pifProtocol_AttachComm(&s_protocol, &s_serial);
    s_protocol.evt_error = _evtProtocolError;

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
