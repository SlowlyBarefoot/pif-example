#include "app_main.h"

#include "sensor/pif_imu_sensor.h"
#include "sensor/pif_qmc5883.h"

#include <math.h>


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

static PifImuSensor s_imu_sensor;
static PifQmc5883 s_qmc5883;


static uint32_t _taskQmc5883(PifTask *pstTask)
{
	float buf[3];

    if (!pifImuSensor_ReadRawMag(&s_imu_sensor, buf)) return 0;
	pifLog_Printf(LT_INFO, "%d %d %d", (int)buf[0], (int)buf[1], (int)buf[2]);
	return 0;
}

BOOL appSetup()
{
    pifImuSensor_Init(&s_imu_sensor);

    if (!pifQmc5883_Init(&s_qmc5883, PIF_ID_AUTO, &g_i2c_port, NULL, &s_imu_sensor)) return false;
#ifdef USE_I2C_WIRE
    s_qmc5883._p_i2c->max_transfer_size = 32;
#endif

    pifQmc5883_SetControl1(&s_qmc5883, QMC5883_MODE_CONTIMUOUS | QMC5883_ODR_200HZ | QMC5883_RNG_8G | QMC5883_OSR_512);

    if (!pifTaskManager_Add(TM_PERIOD, 500000, _taskQmc5883, NULL, TRUE)) return FALSE;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;								// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}
