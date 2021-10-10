#include "appMain.h"
#include "exAds1115.h"

#include "pifAds1x1x.h"
#include "pifLed.h"
#include "pifLog.h"


#define SINGLE_SHOT


PifPulse *g_pstTimer1ms = NULL;

static PIF_stAds1x1x *s_pstAds1x1x = NULL;
static PifLed *s_pstLedL = NULL;


uint16_t _taskAds1115(PifTask *pstTask)
{
	static int channel = ACM_enSINGLE_0;

	(void)pstTask;

#ifdef SINGLE_SHOT
	uint16_t usData = pifAds1x1x_ReadMux(s_pstAds1x1x, (PIF_enAds1x1xConfigMux)channel);
	pifLog_Printf(LT_enInfo, "ADC(%d): %u, Vol: %f", channel, usData, usData * s_pstAds1x1x->dConvertVoltage);
	if (channel == ACM_enSINGLE_3) channel = ACM_enSINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(s_pstAds1x1x);
	pifLog_Printf(LT_enInfo, "ADC: %u, Vol: %f", usData, usData * s_pstAds1x1x->dConvertVoltage);
#endif
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;
	PIF_stAds1x1xConfig stConfig;

    pif_Init(act_timer1us);
    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000);								// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_RATIO, 100, TRUE)) return;			// 100%

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstAds1x1x = pifAds1x1x_Create(PIF_ID_AUTO, AT_en1115);
    if (!s_pstAds1x1x) return;
    pifI2c_AttachAction(&s_pstAds1x1x->_stI2c, actAds1115Read, actAds1115Write);

    stConfig = pifAds1x1x_GetConfig(s_pstAds1x1x);
#if 1
    stConfig.bt.MUX = ACM_enSINGLE_3;
    stConfig.bt.PGA = ACP_enFSR_6_144V;
#ifdef SINGLE_SHOT
    stConfig.bt.MODE = ACM_enSINGLE_SHOT;
#else
    stConfig.bt.MODE = ACM_enCONTINUOUS;
#endif
    stConfig.bt.DR = ACD_enDR_16B_0128_SPS;
    pifAds1x1x_SetConfig(s_pstAds1x1x, &stConfig);
#else
    pifAds1x1x_SetMux(s_pstAds1x1x, ACM_enSINGLE_3);
    pifAds1x1x_SetGain(s_pstAds1x1x, ACP_enFSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(s_pstAds1x1x, ACM_enSINGLE_SHOT);
#else
    pifAds1x1x_SetMode(s_pstAds1x1x, ACM_enCONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(s_pstAds1x1x, ACD_enDR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(s_pstAds1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(s_pstAds1x1x, 2.0);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskAds1115, NULL, TRUE)) return;	// 1000ms
}