#include "app_main.h"

#include "sensor/pif_gy86.h"
#include "sensor/pif_imu_sensor.h"

#include <math.h>


PifI2cPort g_i2c_port;
PifLed g_led_l;
PifTimerManager g_timer_1ms;

static PifGy86 s_gy86;
static PifImuSensor s_imu_sensor;


static double getSeaLevel(double pressure, double altitude)
{
    return ((double)pressure / pow(1.0f - ((double)altitude / 44330.0f), 5.255f));
}

static double getAltitude(double pressure, double seaLevelPressure)
{
    return (44330.0f * (1.0f - pow((double)pressure / (double)seaLevelPressure, 0.1902949f)));
}

static uint16_t _taskMpu60x0(PifTask *pstTask)
{
	static uint8_t step = 0;
	float gyro[3], accel[3], mag[3];

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
		step++;
		break;

	case 4:
		if (pifImuSensor_ReadRawMag(&s_imu_sensor, mag)) {
			pifLog_Printf(LT_NONE, "\nMagR:\t%f\t%f\t%f", mag[0], mag[1], mag[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nMagR:\tError");
		}
		step++;
		break;

	case 5:
		if (pifImuSensor_ReadMag(&s_imu_sensor, mag)) {
			pifLog_Printf(LT_NONE, "\nMagN:\t%f\t%f\t%f", mag[0], mag[1], mag[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nMagN:\tError");
		}
		step = 0;
		break;
	}
	return 0;
}

static void _evtBaroRead(float pressure, float temperature)
{
	pifLog_Printf(LT_NONE, "\nBaro Temp : %2f DegC", temperature);
	pifLog_Printf(LT_NONE, "\nBaro : %2f hPa, %f m", pressure, getAltitude(pressure, 103100));
}

BOOL appSetup()
{
	PifGy86Param* p_param;

    pifImuSensor_Init(&s_imu_sensor);

    p_param = pifGy86_InitParam();
    p_param->mpu60x0_clksel = MPU60X0_CLKSEL_PLL_ZGYRO;
    p_param->mpu60x0_dlpf_cfg = MPU60X0_DLPF_CFG_A10HZ_G10HZ;
    p_param->mpu60x0_fs_sel = MPU60X0_FS_SEL_2000DPS;
    p_param->mpu60x0_afs_sel = MPU60X0_AFS_SEL_8G;
    p_param->mpu60x0_i2c_mst_clk = MPU60X0_I2C_MST_CLK_400KHZ;
    p_param->hmc5883_gain = HMC5883_GAIN_1_3GA;
    p_param->hmc5883_samples = HMC5883_SAMPLES_8;
    p_param->hmc5883_data_rate = HMC5883_DATARATE_75HZ;
    p_param->hmc5883_mode = HMC5883_MODE_CONTINOUS;
    p_param->ms5611_osr = MS5611_OSR_4096;
    p_param->ms5611_read_period = 2000;													// 2000ms
    p_param->ms5611_evt_read = _evtBaroRead;
    if (!pifGy86_Init(&s_gy86, PIF_ID_AUTO, &g_i2c_port, p_param, &s_imu_sensor)) return FALSE;
    s_gy86._mpu6050.temp_scale = 100;
    s_gy86._ms5611._p_task->pause = FALSE;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskMpu60x0, NULL, TRUE)) return FALSE;	// 500ms

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;								// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);
    return TRUE;
}

