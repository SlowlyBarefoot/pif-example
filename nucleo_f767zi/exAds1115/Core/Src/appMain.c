#include "appMain.h"
#include "main.h"

#include "pifLed.h"
#include "pifLog.h"


#define ADS1X1X_COUNT         	1
#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              3

#define SINGLE_SHOT


PIF_stAds1x1x *g_pstAds1x1x = NULL;
PIF_stComm *g_pstCommLog = NULL;
PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stLed *s_pstLedL = NULL;


uint16_t _taskAds1115(PIF_stTask *pstTask)
{
	(void)pstTask;

#ifdef SINGLE_SHOT
	static int channel = ACM_enSINGLE_0;

	uint16_t usData = pifAds1x1x_ReadMux(g_pstAds1x1x, (PIF_enAds1x1xConfigMux)channel);
	pifLog_Printf(LT_enInfo, "ADC(%d): %u, Vol: %f", channel, usData, usData * g_pstAds1x1x->dConvertVoltage);
	if (channel == ACM_enSINGLE_3) channel = ACM_enSINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(g_pstAds1x1x);
	pifLog_Printf(LT_enInfo, "ADC: %u, Vol: %f", usData, usData * g_pstAds1x1x->dConvertVoltage);
#endif
	return 0;
}

void appSetup(PIF_actTimer1us actTimer1us)
{
	PIF_stAds1x1xConfig stConfig;

    pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifAds1x1x_Init(ADS1X1X_COUNT)) return;
    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);			// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifPulse_AttachTask(g_pstTimer1ms, TM_enRatio, 100, TRUE)) return;		// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    g_pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!g_pstCommLog) return;
    if (!pifComm_AttachTask(g_pstCommLog, TM_enPeriodMs, 1, TRUE)) return;		// 1ms
	if (!pifComm_AllocTxBuffer(g_pstCommLog, 64)) return;
	pifComm_AttachActStartTransfer(g_pstCommLog, actLogStartTransfer);

	if (!pifLog_AttachComm(g_pstCommLog)) return;

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;							// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    g_pstAds1x1x = pifAds1x1x_Add(PIF_ID_AUTO, AT_en1115);
    pifI2c_AttachAction(&g_pstAds1x1x->_stI2c, actAds1115Read, actAds1115Write);

    stConfig = pifAds1x1x_GetConfig(g_pstAds1x1x);
#if 1
    stConfig.bt.MUX = ACM_enSINGLE_3;
    stConfig.bt.PGA = ACP_enFSR_6_144V;
#ifdef SINGLE_SHOT
    stConfig.bt.MODE = ACM_enSINGLE_SHOT;
#else
    stConfig.bt.MODE = ACM_enCONTINUOUS;
#endif
    stConfig.bt.DR = ACD_enDR_16B_0128_SPS;
    pifAds1x1x_SetConfig(g_pstAds1x1x, &stConfig);
#else
    pifAds1x1x_SetMux(g_pstAds1x1x, ACM_enSINGLE_3);
    pifAds1x1x_SetGain(g_pstAds1x1x, ACP_enFSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(g_pstAds1x1x, ACM_enSINGLE_SHOT);
#else
    pifAds1x1x_SetMode(g_pstAds1x1x, ACM_enCONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(g_pstAds1x1x, ACD_enDR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(g_pstAds1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(g_pstAds1x1x, 2.0);

    if (!pifTask_Add(TM_enPeriodMs, 1000, _taskAds1115, NULL, TRUE)) return;	// 1000ms
}
