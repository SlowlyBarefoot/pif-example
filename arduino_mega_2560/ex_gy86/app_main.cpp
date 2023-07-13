#include "app_main.h"
#include "ex_gy86.h"

#include "core/pif_i2c.h"
#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_imu_sensor.h"
#include "sensor/pif_gy86.h"


PifTimerManager g_timer_1ms;

static PifGy86 s_gy86;
static PifI2cPort s_i2c_port;
static PifImuSensor s_imu_sensor;
static PifLed s_led_l;


double getSeaLevel(double pressure, double altitude)
{
    return ((double)pressure / pow(1.0f - ((double)altitude / 44330.0f), 5.255f));
}

double getAltitude(double pressure, double seaLevelPressure)
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

void _evtBaroRead(float pressure, float temperature)
{
	pifLog_Printf(LT_NONE, "\nBaro Temp : %2f DegC", temperature);
	pifLog_Printf(LT_NONE, "\nBaro : %2f hPa, %f m", pressure, getAltitude(pressure, 103100));
}

void appSetup(PifActTimer1us act_timer1us)
{
	static PifComm s_comm_log;
	PifGy86Param param;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(5)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&s_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&s_comm_log, TM_PERIOD_MS, 1, NULL)) return;		// 1ms
    s_comm_log.act_send_data = actLogSendData;

	if (!pifLog_AttachComm(&s_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    if (!pifI2cPort_Init(&s_i2c_port, PIF_ID_AUTO, 3, 16)) return;
    s_i2c_port.act_read = actI2cRead;
    s_i2c_port.act_write = actI2cWrite;

    pifImuSensor_Init(&s_imu_sensor);

    param.mpu60x0_clksel = MPU60X0_CLKSEL_PLL_ZGYRO;
    param.mpu60x0_dlpf_cfg = MPU60X0_DLPF_CFG_A10HZ_G10HZ;
    param.mpu60x0_fs_sel = MPU60X0_FS_SEL_2000DPS;
    param.mpu60x0_afs_sel = MPU60X0_AFS_SEL_8G;
    param.mpu60x0_i2c_mst_clk = MPU60X0_I2C_MST_CLK_400KHZ;
    param.hmc5883_gain = HMC5883_GAIN_1_3GA;
    param.hmc5883_samples = HMC5883_SAMPLES_8;
    param.hmc5883_data_rate = HMC5883_DATARATE_75HZ;
    param.hmc5883_mode = HMC5883_MODE_CONTINOUS;
    param.ms5611_osr = MS5611_OSR_4096;
    param.ms5611_read_period = 2000;												// 2000ms
    param.ms5611_evt_read = _evtBaroRead;
    if (!pifGy86_Init(&s_gy86, PIF_ID_AUTO, &s_i2c_port, &param, &s_imu_sensor)) return;
    s_gy86._mpu6050.temp_scale = 100;
    s_gy86._ms5611._p_task->pause = FALSE;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskMpu60x0, NULL, TRUE)) return;	// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

