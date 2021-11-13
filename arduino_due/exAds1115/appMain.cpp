#include "appMain.h"
#include "exAds1115.h"

#include "pif_ads1x1x.h"
#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifPulse *g_pstTimer1ms = NULL;

static PifAds1x1x *s_pstAds1x1x = NULL;
static PifLed *s_pstLedL = NULL;


uint16_t _taskAds1115(PifTask *pstTask)
{
	static int channel = ACM_SINGLE_0;

	(void)pstTask;

#ifdef SINGLE_SHOT
	uint16_t usData = pifAds1x1x_ReadMux(s_pstAds1x1x, (PifAds1x1xConfigMux)channel);
	pifLog_Printf(LT_INFO, "ADC(%d): %u, Vol: %f", channel, usData, usData * s_pstAds1x1x->convert_voltage);
	if (channel == ACM_SINGLE_3) channel = ACM_SINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(s_pstAds1x1x);
	pifLog_Printf(LT_INFO, "ADC: %u, Vol: %f", usData, usData * s_pstAds1x1x->convert_voltage);
#endif
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifComm *pstCommLog;
	PifAds1x1xConfig stConfig;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    g_pstTimer1ms = pifPulse_Create(PIF_ID_AUTO, 1000, 1);							// 1000us
    if (!g_pstTimer1ms) return;

    pstCommLog = pifComm_Create(PIF_ID_AUTO);
	if (!pstCommLog) return;
    if (!pifComm_AttachTask(pstCommLog, TM_PERIOD_MS, 1, TRUE)) return;				// 1ms
	pstCommLog->act_send_data = actLogSendData;

	if (!pifLog_AttachComm(pstCommLog)) return;

    s_pstLedL = pifLed_Create(PIF_ID_AUTO, g_pstTimer1ms, 1, actLedLState);
    if (!s_pstLedL) return;
    if (!pifLed_AttachBlink(s_pstLedL, 500)) return;								// 500ms

    s_pstAds1x1x = pifAds1x1x_Create(PIF_ID_AUTO, AT_1115);
    if (!s_pstAds1x1x) return;
    s_pstAds1x1x->_i2c.act_read = actAds1115Read;
    s_pstAds1x1x->_i2c.act_write = actAds1115Write;

    stConfig = pifAds1x1x_GetConfig(s_pstAds1x1x);
#if 1
    stConfig.bt.mux = ACM_SINGLE_3;
    stConfig.bt.pga = ACP_FSR_6_144V;
#ifdef SINGLE_SHOT
    stConfig.bt.mode = ACM_SINGLE_SHOT;
#else
    stConfig.bt.mode = ACM_CONTINUOUS;
#endif
    stConfig.bt.dr = ACD_DR_16B_0128_SPS;
    pifAds1x1x_SetConfig(s_pstAds1x1x, &stConfig);
#else
    pifAds1x1x_SetMux(s_pstAds1x1x, ACM_SINGLE_3);
    pifAds1x1x_SetGain(s_pstAds1x1x, ACP_FSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(s_pstAds1x1x, ACM_SINGLE_SHOT);
#else
    pifAds1x1x_SetMode(s_pstAds1x1x, ACM_CONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(s_pstAds1x1x, ACD_DR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(s_pstAds1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(s_pstAds1x1x, 2.0);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskAds1115, NULL, TRUE)) return;	// 1000ms

    pifLed_BlinkOn(s_pstLedL, 0);

	pifLog_Printf(LT_INFO, "Task=%d Pulse=%d\n", pifTaskManager_Count(), pifPulse_Count(g_pstTimer1ms));
}
