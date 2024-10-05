#include "app_main.h"

#include "sensor/pif_bmp280_i2c.h"
#include "sensor/pif_mpu6500_i2c.h"
#include "sensor/pif_imu_sensor.h"

#include <math.h>


#define USE_BARO_TASK

#define SEA_LEVEL		1020


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

static PifBmp280 s_bmp280;
static PifMpu6500 s_mpu6500;
static PifImuSensor s_imu_sensor;


static double getSeaLevel(double pressure, double altitude)
{
    return ((double)pressure / pow(1.0f - ((double)altitude / 44330.0f), 5.255f));
}

static double getAltitude(double pressure, double seaLevelPressure)
{
    return (44330.0f * (1.0f - pow((double)pressure / (double)seaLevelPressure, 0.1902949f)));
}

static uint16_t _taskMpu6500(PifTask *pstTask)
{
	static uint8_t step = 0;
	float gyro[3], accel[3];
#ifndef USE_BARO_TASK
	float pressure, temperature;
#endif

	(void)pstTask;

	switch (step) {
	case 0:
		pifLog_Printf(LT_NONE, "\n\n\tX\t\tY\t\tZ");
		if (pifImuSensor_ReadRawGyro(&s_imu_sensor, gyro)) {
			pifLog_Printf(LT_NONE, "\nGyroR:\t%f\t%f\t%f", gyro[0], gyro[1], gyro[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nGyroR:\tError");
		}
		step++;
		break;

	case 1:
		if (pifImuSensor_ReadGyro(&s_imu_sensor, gyro)) {
			pifLog_Printf(LT_NONE, "\nGyroN:\t%f\t%f\t%f", gyro[0], gyro[1], gyro[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nGyroN:\tError");
		}
		step++;
		break;

	case 2:
		if (pifImuSensor_ReadRawAccel(&s_imu_sensor, accel)) {
			pifLog_Printf(LT_NONE, "\nAccelR:\t%f\t%f\t%f", accel[0], accel[1], accel[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nAccelR:\tError");
		}
		step++;
		break;

	case 3:
		if (pifImuSensor_ReadAccel(&s_imu_sensor, accel)) {
			pifLog_Printf(LT_NONE, "\nAccelN:\t%f\t%f\t%f", accel[0], accel[1], accel[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nAccelN:\tError");
		}
#ifndef USE_BARO_TASK
		step++;
		break;

	case 4:
		if (pifBmp280_ReadBarometric(&s_bmp280, &pressure, &temperature)) {
			pifLog_Printf(LT_NONE, "\nBaro Temp : %2f DegC", temperature);
			pifLog_Printf(LT_NONE, "\nBaro : %2f hPa, %f m, %2f hPa", pressure, getAltitude(pressure, SEA_LEVEL), getSeaLevel(pressure, 100));
		}
		else {
			pifLog_Printf(LT_NONE, "\nBaro:\tError");
		}
#endif
		step = 0;
		break;
	}
	return 0;
}

#ifdef USE_BARO_TASK

static void _evtBaroRead(float pressure, float temperature)
{
	pifLog_Printf(LT_NONE, "\nBaro Temp : %2f DegC", temperature);
	pifLog_Printf(LT_NONE, "\nBaro : %2f hPa, %f m, %2f hPa", pressure, getAltitude(pressure, SEA_LEVEL), getSeaLevel(pressure, 100));
}

#endif

BOOL appSetup()
{
    pifImuSensor_Init(&s_imu_sensor);

	if (!pifMpu6500I2c_Detect(&g_i2c_port, MPU6500_I2C_ADDR(0))) return FALSE;

	if (!pifBmp280I2c_Detect(&g_i2c_port, BMP280_I2C_ADDR(0))) return FALSE;

	if (!pifMpu6500I2c_Init(&s_mpu6500, PIF_ID_AUTO, &g_i2c_port, MPU6500_I2C_ADDR(0), &s_imu_sensor)) return FALSE;

	if (!pifBmp280I2c_Init(&s_bmp280, PIF_ID_AUTO, &g_i2c_port, BMP280_I2C_ADDR(0))) return FALSE;

	pifBmp280_SetOverSamplingRate(&s_bmp280, BMP280_OSRS_P_X16, BMP280_OSRS_T_X2);
#ifdef USE_BARO_TASK
	if (!pifBmp280_AddTaskForReading(&s_bmp280, 2000, &_evtBaroRead, TRUE)) return FALSE;		// 2sec
#endif

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskMpu6500, NULL, TRUE)) return FALSE;			// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;										// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

