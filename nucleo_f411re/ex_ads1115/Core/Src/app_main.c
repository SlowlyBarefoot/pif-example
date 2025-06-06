#include "linker.h"

#include "sensor/pif_ads1x1x.h"


#define SINGLE_SHOT


PifLed g_led_l;
PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;

static PifAds1x1x s_ads1x1x;

static int channel = ADS1X1X_MUX_SINGLE_0;


uint32_t _taskAds1115(PifTask *p_task)
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

BOOL appSetup()
{
	uint16_t config;

    if (!pifAds1x1x_Init(&s_ads1x1x, PIF_ID_AUTO, ADS1X1X_TYPE_1115, &g_i2c_port, ADS1X1X_I2C_ADDR(0), NULL)) return FALSE;

    config = s_ads1x1x._config;
#if 1
    SET_BIT_FILED(config, ADS1X1X_MUX_MASK, channel);
    SET_BIT_FILED(config, ADS1X1X_PGA_MASK, ADS1X1X_PGA_FSR_6_144V);
#ifdef SINGLE_SHOT
    SET_BIT_FILED(config, ADS1X1X_MODE_MASK, ADS1X1X_MODE_SINGLE_SHOT);
#else
    SET_BIT_FILED(config, ADS1X1X_MODE_MASK, ADS1X1X_MODE_CONTINUOUS);
#endif
    SET_BIT_FILED(config, ADS1X1X_DR_MASK, ADS1X1X_DR_16B_0128_SPS);
    pifAds1x1x_SetConfig(&s_ads1x1x, config);
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

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, _taskAds1115, NULL, TRUE)) return FALSE;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;												// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
