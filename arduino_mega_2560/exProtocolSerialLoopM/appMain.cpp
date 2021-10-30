#include "appMain.h"
#include "exProtocolSerialLoopM.h"

#include "pif_led.h"
#include "pif_log.h"
#include "pif_protocol.h"
#include "pif_sensor_switch.h"

#include "exSerial1.h"
#include "exSerial2.h"


PifPulse *g_pstTimer1ms = NULL;
PifComm *g_pstCommLog = NULL;


void appSetup()
{
	PifLed *pstLedL;

	pif_Init(NULL);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);						// 1000us
    if (!g_pstTimer1ms) return;

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;	// 1ms
#ifdef USE_SERIAL
	g_pstCommLog->act_send_data = actLogSendData;
#endif
#ifdef USE_USART
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	g_pstCommLog->act_start_transfer = actLogStartTransfer;
#endif

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!pstLedL) return;
    if (!pifLed_AttachBlink(pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(pstLedL, 0);

    if (!exSerial1_Setup()) return;
    if (!exSerial2_Setup()) return;
}

