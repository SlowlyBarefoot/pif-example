#include "appMain.h"
#include "exAds1115.h"

#include "pifAds1x1x.h"
#include "pifLed.h"
#include "pifLog.h"


#define ADS1X1X_COUNT         	1
#define COMM_COUNT         		1
#define LED_COUNT         		1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define TASK_COUNT              3

#define SINGLE_SHOT


PIF_stPulse *g_pstTimer1ms = NULL;

static PIF_stAds1x1x *s_pstAds1x1x = NULL;
static PIF_stLed *s_pstLedL = NULL;


uint16_t _taskAds1115(PIF_stTask *pstTask)
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

void appSetup(PIF_actTimer1us actTimer1us)
{
	PIF_stComm *pstCommLog;
	PIF_stAds1x1xConfig stConfig;

    pif_Init(actTimer1us);
    pifLog_Init();

    if (!pifAds1x1x_Init(ADS1X1X_COUNT)) return;
    if (!pifComm_Init(COMM_COUNT)) return;
    if (!pifPulse_Init(PULSE_COUNT)) return;
    if (!pifTask_Init(TASK_COUNT)) return;

    g_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!g_pstTimer1ms) return;
    if (!pifTask_AddRatio(100, pifPulse_Task, g_pstTimer1ms, TRUE)) return;	// 100%

    if (!pifLed_Init(LED_COUNT, g_pstTimer1ms)) return;

    pstCommLog = pifComm_Add(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifTask_AddPeriodMs(1, pifComm_Task, pstCommLog, TRUE)) return;	// 1ms
	pifComm_AttachActSendData(pstCommLog, actLogSendData);

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Add(PIF_ID_AUTO, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;						// 500ms
    pifLed_BlinkOn(s_pstLedL, 0);

    s_pstAds1x1x = pifAds1x1x_Add(PIF_ID_AUTO, AT_en1115);
    pifAds1x1x_AttachAction(s_pstAds1x1x, actAds1115StartRead, actAds1115StartWrite);

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

    if (!pifTask_AddPeriodMs(1000, _taskAds1115, NULL, TRUE)) return;		// 1000ms
}
