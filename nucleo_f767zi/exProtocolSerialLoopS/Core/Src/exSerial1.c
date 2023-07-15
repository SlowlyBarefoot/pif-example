#include <stdlib.h>

#include "main.h"
#include "appMain.h"

#include "core/pif_log.h"
#include "filter/pif_noise_filter_bit.h"
#include "protocol/pif_protocol.h"
#include "sensor/pif_sensor_switch.h"


PifUart g_serial1;

static PifProtocol s_protocol;

static void _fnProtocolAnswer20(PifProtocolPacket *pstPacket);
static void _fnProtocolAnswer21(PifProtocolPacket *pstPacket);

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket);
static void _fnProtocolResponse31(PifProtocolPacket *pstPacket);

const PifProtocolQuestion stProtocolQuestions1[] = {
		{ 0x20, PF_ANSWER_YES | PF_LOG_PRINT_YES, _fnProtocolAnswer20 },
		{ 0x21, PF_ANSWER_NO | PF_LOG_PRINT_YES, _fnProtocolAnswer21 },
		{ 0, PF_DEFAULT, NULL }
};

const PifProtocolRequest stProtocolRequests1[] = {
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
		pifLog_Printf(LT_INFO, "%s: CNT=%u", pcName, pstPacket->data_count);
#ifdef PRINT_PACKET_DATA
		if (pstPacket->data_count) {
			pifLog_Printf(LT_NONE, "\nData:");
			for (int i = 0; i < pstPacket->data_count; i++) {
				pifLog_Printf(LT_NONE, " %u", pstPacket->p_data[i]);
			}
		}
#endif
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
	}
	else {
		pifLog_Printf(LT_ERROR, "Different count: %u != %u", s_stProtocolTest[ucIndex].ucDataCount, pstPacket->data_count);
	}
}

static void _fnProtocolAnswer20(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 0);
	_fnProtocolPrint(pstPacket, "Answer20");

	if (!pifProtocol_MakeAnswer(&s_protocol, pstPacket, stProtocolQuestions1[0].flags, NULL, 0)) {
		pifLog_Printf(LT_INFO, "Answer20: Error=%d", pif_error);
	}
}

static void _fnProtocolAnswer21(PifProtocolPacket *pstPacket)
{
	_fnCompareData(pstPacket, 1);
	_fnProtocolPrint(pstPacket, "Answer21");
}

static void _fnProtocolResponse30(PifProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response30");
}

static void _fnProtocolResponse31(PifProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_INFO, "Response31");
}

static void _evtProtocolError(PifId usPifId)
{
	pifLog_Printf(LT_ERROR, "ProtocolError DC=%d", usPifId);
}

static void _evtPushSwitchChange(PifSensor* p_owner, SWITCH state, PifSensorValueP p_value, PifIssuerP p_issuer)
{
	uint8_t index = p_owner->_id - PIF_ID_SWITCH;

	(void)p_value;
	(void)p_issuer;

	if (state) {
		s_stProtocolTest[index].ucDataCount = rand() % 8;
		for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) s_stProtocolTest[index].ucData[i] = rand() & 0xFF;
		if (!pifProtocol_MakeRequest(&s_protocol, &stProtocolRequests1[index], s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
			pifLog_Printf(LT_ERROR, "PSC(%d): DC=%d E=%d", index, s_protocol._id, pif_error);
		}
		else {
			pifLog_Printf(LT_INFO, "PSC(%d): DC=%d CNT=%u", index, s_protocol._id, s_stProtocolTest[index].ucDataCount);
#ifdef PRINT_PACKET_DATA
			if (s_stProtocolTest[index].ucDataCount) {
				pifLog_Printf(LT_NONE, "\nData:");
				for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
					pifLog_Printf(LT_NONE, " %u", s_stProtocolTest[index].ucData[i]);
				}
			}
#endif
		}
	}
}

BOOL exSerial1_Setup()
{
	int i;

    for (i = 0; i < SWITCH_COUNT; i++) {
	    if (!pifNoiseFilterBit_Init(&s_stProtocolTest[i].stPushSwitchFilter, 7)) return FALSE;

	    if (!pifSensorSwitch_Init(&s_stProtocolTest[i].stPushSwitch, PIF_ID_SWITCH + i, 0, actPushSwitchAcquire, NULL)) return FALSE;
	    if (!pifSensorSwitch_AttachTaskAcquire(&s_stProtocolTest[i].stPushSwitch, TM_PERIOD_MS, 10, TRUE)) return FALSE;	// 10ms
		pifSensor_AttachEvtChange(&s_stProtocolTest[i].stPushSwitch.parent, _evtPushSwitchChange);
		s_stProtocolTest[i].stPushSwitch.p_filter = &s_stProtocolTest[i].stPushSwitchFilter.parent;
    	s_stProtocolTest[i].ucDataCount = 0;
    }

	if (!pifUart_Init(&g_serial1, PIF_ID_AUTO)) return FALSE;
    if (!pifUart_AttachTask(&g_serial1, TM_PERIOD_MS, 1, "UartSerial1")) return FALSE;										// 1ms
	if (!pifUart_AllocRxBuffer(&g_serial1, 64, 10)) return FALSE;															// 10%
	if (!pifUart_AllocTxBuffer(&g_serial1, 64)) return FALSE;
	g_serial1.act_start_transfer = actUart1StartTransfer;

    if (!pifProtocol_Init(&s_protocol, PIF_ID_AUTO, &g_timer_1ms, PT_SMALL, stProtocolQuestions1)) return FALSE;
#ifdef USE_DMA
    if (!pifProtocol_SetFrameSize(&s_protocol, UART_FRAME_SIZE)) return FALSE;
#endif
    pifProtocol_AttachUart(&s_protocol, &g_serial1);
    s_protocol.evt_error = _evtProtocolError;

    return TRUE;
}
