#include "appMain.h"
#include "exAds1115.h"

#include "pif_ads1x1x.h"
#include "pif_led.h"
#include "pif_log.h"


#define SINGLE_SHOT


PifTimerManager g_timer_1ms;

static PifAds1x1x s_ads1x1x;
static PifI2cPort s_i2c_port;
static PifLed s_led_l;

static int channel = ACM_SINGLE_0;


uint16_t _taskAds1115(PifTask *pstTask)
{
	(void)pstTask;

#ifdef SINGLE_SHOT
	uint16_t usData = pifAds1x1x_ReadMux(&s_ads1x1x, (PifAds1x1xConfigMux)channel);
	pifLog_Printf(LT_INFO, "ADC(%d): %u, Vol: %f", channel, usData, usData * s_ads1x1x.convert_voltage);
	if (channel == ACM_SINGLE_3) channel = ACM_SINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(&s_ads1x1x);
	pifLog_Printf(LT_INFO, "ADC: %u, Vol: %f", usData, usData * s_ads1x1x.convert_voltage);
#endif
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	PifAds1x1xConfig stConfig;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachBlink(&s_led_l, 500)) return;									// 500ms

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 1)) return;
    s_i2c_port.act_read = actI2cRead;
    s_i2c_port.act_write = actI2cWrite;

    if (!pifAds1x1x_Init(&s_ads1x1x, PIF_ID_AUTO, AT_1115, &s_i2c_port)) return;

    stConfig = pifAds1x1x_GetConfig(&s_ads1x1x);
#if 1
    stConfig.bt.mux = channel;
    stConfig.bt.pga = ACP_FSR_6_144V;
#ifdef SINGLE_SHOT
    stConfig.bt.mode = ACM_SINGLE_SHOT;
#else
    stConfig.bt.mode = ACM_CONTINUOUS;
#endif
    stConfig.bt.dr = ACD_DR_16B_0128_SPS;
    pifAds1x1x_SetConfig(&s_ads1x1x, &stConfig);
#else
    pifAds1x1x_SetMux(&s_ads1x1x, ACM_SINGLE_3);
    pifAds1x1x_SetGain(&s_ads1x1x, ACP_FSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(&s_ads1x1x, ACM_SINGLE_SHOT);
#else
    pifAds1x1x_SetMode(&s_ads1x1x, ACM_CONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(&s_ads1x1x, ACD_DR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(&s_ads1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(&s_ads1x1x, 2.0);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 1000, _taskAds1115, NULL, TRUE)) return;	// 1000ms

    pifLed_BlinkOn(&s_led_l, 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
