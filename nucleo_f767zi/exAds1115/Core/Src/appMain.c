#include "appMain.h"
#include "main.h"

#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifAds1x1x *g_pstAds1x1x = NULL;
PifComm *g_pstCommLog = NULL;
PifPulse *g_pstTimer1ms = NULL;

static PifLed *s_pstLedL = NULL;


uint16_t _taskAds1115(PifTask *pstTask)
{
	(void)pstTask;

#ifdef SINGLE_SHOT
	static int channel = ACM_SINGLE_0;

	uint16_t usData = pifAds1x1x_ReadMux(g_pstAds1x1x, (PifAds1x1xConfigMux)channel);
	pifLog_Printf(LT_INFO, "ADC(%d): %u, Vol: %f", channel, usData, usData * g_pstAds1x1x->convert_voltage);
	if (channel == ACM_SINGLE_3) channel = ACM_SINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(g_pstAds1x1x);
	pifLog_Printf(LT_INFO, "ADC: %u, Vol: %f", usData, usData * g_pstAds1x1x->convert_voltage);
#endif
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifAds1x1xConfig stConfig;

    pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;

    g_pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    g_pstAds1x1x = pifAds1x1x_Create(PIF_ID_AUTO, AT_1115);
    if (!g_pstAds1x1x) return;
    pifI2c_AttachAction(&g_pstAds1x1x->_i2c, actAds1115Read, actAds1115Write);

    stConfig = pifAds1x1x_GetConfig(g_pstAds1x1x);
#if 1
    stConfig.bt.mux = ACM_SINGLE_3;
    stConfig.bt.pga = ACP_FSR_6_144V;
#ifdef SINGLE_SHOT
    stConfig.bt.mode = ACM_SINGLE_SHOT;
#else
    stConfig.bt.mode = ACM_CONTINUOUS;
#endif
    stConfig.bt.dr = ACD_DR_16B_0128_SPS;
    pifAds1x1x_SetConfig(g_pstAds1x1x, &stConfig);
#else
    pifAds1x1x_SetMux(g_pstAds1x1x, ACM_SINGLE_3);
    pifAds1x1x_SetGain(g_pstAds1x1x, ACP_FSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(g_pstAds1x1x, ACM_SINGLE_SHOT);
#else
    pifAds1x1x_SetMode(g_pstAds1x1x, ACM_CONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(g_pstAds1x1x, ACD_DR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(g_pstAds1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(g_pstAds1x1x, 2.0);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskAds1115, NULL, TRUE)) return;	// 1000ms
}
