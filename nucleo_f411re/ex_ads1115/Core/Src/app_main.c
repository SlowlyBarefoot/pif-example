#include "linker.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_ads1x1x.h"


#define SINGLE_SHOT


PifComm g_comm_log;
PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;

static PifAds1x1x s_ads1x1x;
static PifLed s_led_l;

static int channel = ADS1X1X_MUX_SINGLE_0;


uint16_t _taskAds1115(PifTask *p_task)
{
	(void)p_task;

#ifdef SINGLE_SHOT
	uint16_t usData = pifAds1x1x_ReadMux(&s_ads1x1x, (PifAds1x1xMux)channel);
	pifLog_Printf(LT_INFO, "ADC(%d): %5u, Vol: %f", channel, usData, usData * s_ads1x1x.convert_voltage);
	if (channel == ADS1X1X_MUX_SINGLE_3) channel = ADS1X1X_MUX_SINGLE_0; else channel++;
#else
	uint16_t usData = pifAds1x1x_Read(&s_ads1x1x);
	pifLog_Printf(LT_INFO, "ADC: %u, Vol: %f", usData, usData * s_ads1x1x.convert_voltage);
#endif
	return 0;
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifAds1x1xConfig config;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(3)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, NULL)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 64)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 1, 16)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

    if (!pifAds1x1x_Init(&s_ads1x1x, PIF_ID_AUTO, ADS1X1X_TYPE_1115, &g_i2c_port, ADS1X1X_I2C_ADDR(0))) return;

    config = s_ads1x1x._config;
#if 1
    config.bit.mux = channel;
    config.bit.pga = ADS1X1X_PGA_FSR_6_144V;
#ifdef SINGLE_SHOT
    config.bit.mode = ADS1X1X_MODE_SINGLE_SHOT;
#else
    config.bit.mode = ADS1X1X_MODE_CONTINUOUS;
#endif
    config.bit.dr = ADS1X1X_DR_16B_0128_SPS;
    pifAds1x1x_SetConfig(&s_ads1x1x, &config);
#else
    pifAds1x1x_SetMux(&s_ads1x1x, ADS1X1X_MUX_SINGLE_3);
    pifAds1x1x_SetGain(&s_ads1x1x, ADS1X1X_PGA_FSR_6_144V);
#ifdef SINGLE_SHOT
    pifAds1x1x_SetMode(&s_ads1x1x, ADS1X1X_MODE_SINGLE_SHOT);
#else
    pifAds1x1x_SetMode(&s_ads1x1x, ADS1X1X_MODE_CONTINUOUS);
#endif
    pifAds1x1x_SetDataRate(&s_ads1x1x, ADS1X1X_DR_16B_0128_SPS);
#endif
    pifAds1x1x_SetLoThreshVoltage(&s_ads1x1x, 1.0);
    pifAds1x1x_SetHiThreshVoltage(&s_ads1x1x, 2.0);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskAds1115, NULL, TRUE)) return;	// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}
