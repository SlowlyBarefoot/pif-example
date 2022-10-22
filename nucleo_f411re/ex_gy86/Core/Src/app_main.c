#include "app_main.h"
#include "main.h"

#include "core/pif_log.h"
#include "display/pif_led.h"
#include "sensor/pif_imu_sensor.h"
#include "sensor/pif_gy86.h"

#include <math.h>


PifComm g_comm_log;
PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;

static PifGy86 s_gy86;
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
	int16_t gyro[3], accel[3], mag[3];

	(void)pstTask;

	switch (step) {
	case 0:
		pifLog_Printf(LT_NONE, "\n\n         X      Y      Z");
		if (pifImuSensor_ReadGyro(&s_imu_sensor, gyro)) {
			pifLog_Printf(LT_NONE, "\nGyro : %6d %6d %6d", gyro[0], gyro[1], gyro[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nGyro : Error");
		}
		step++;
		break;

	case 1:
		if (pifImuSensor_ReadAccel(&s_imu_sensor, accel)) {
			pifLog_Printf(LT_NONE, "\nAccel: %6d %6d %6d", accel[0], accel[1], accel[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nAccel: Error");
		}
		step++;
		break;

	case 2:
		if (pifImuSensor_ReadMag(&s_imu_sensor, mag)) {
			pifLog_Printf(LT_NONE, "\nMag  : %6d %6d %6d", mag[0], mag[1], mag[2]);
		}
		else {
			pifLog_Printf(LT_NONE, "\nMag   : Error");
		}
		step = 0;
		break;
	}
	return 0;
}

void _evtBaroRead(int32_t pressure, float temperature)
{
	pifLog_Printf(LT_NONE, "\nBaro Temp : %2f", temperature);
	pifLog_Printf(LT_NONE, "\nBaro : %ld, %f", pressure, getAltitude(pressure, 103100));
}

void appSetup(PifActTimer1us act_timer1us)
{
	PifGy86Config config;

    pif_Init(act_timer1us);

    if (!pifTaskManager_Init(8)) return;

    pifLog_Init();

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 1)) return;			// 1000us

	if (!pifComm_Init(&g_comm_log, PIF_ID_AUTO)) return;
    if (!pifComm_AttachTask(&g_comm_log, TM_PERIOD_MS, 1, TRUE)) return;			// 1ms
	if (!pifComm_AllocTxBuffer(&g_comm_log, 256)) return;
	g_comm_log.act_start_transfer = actLogStartTransfer;

	if (!pifLog_AttachComm(&g_comm_log)) return;

    if (!pifLed_Init(&s_led_l, PIF_ID_AUTO, &g_timer_1ms, 1, actLedLState)) return;
    if (!pifLed_AttachSBlink(&s_led_l, 500)) return;								// 500ms

    if (!pifI2cPort_Init(&g_i2c_port, PIF_ID_AUTO, 3, 16)) return;
    g_i2c_port.act_read = actI2cRead;
    g_i2c_port.act_write = actI2cWrite;

    pifImuSensor_Init(&s_imu_sensor);

    config.mpu60x0_clksel = MPU60X0_CLKSEL_PLL_ZGYRO;
    config.mpu60x0_dlpf_cfg = MPU60X0_DLPF_CFG_A10HZ_G10HZ;
    config.mpu60x0_fs_sel = MPU60X0_FS_SEL_2000DPS;
    config.mpu60x0_afs_sel = MPU60X0_AFS_SEL_8G;
    config.mpu60x0_i2c_mst_clk = MPU60X0_I2C_MST_CLK_400KHZ;
    config.hmc5883_gain = HMC5883_GAIN_1_3GA;
    config.hmc5883_samples = HMC5883_SAMPLES_8;
    config.hmc5883_data_rate = HMC5883_DATARATE_75HZ;
    config.hmc5883_mode = HMC5883_MODE_CONTINOUS;
    config.ms5611_osr = MS5611_OSR_4096;
    config.ms5611_read_period = 2000;												// 2000ms
    config.ms5611_evt_read = _evtBaroRead;
    if (!pifGy86_Init(&s_gy86, PIF_ID_AUTO, &g_i2c_port, &s_imu_sensor, &config)) return;

    s_gy86._mpu6050.temp_scale = 100;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, _taskMpu60x0, NULL, TRUE)) return;	// 500ms

    pifLed_SBlinkOn(&s_led_l, 1 << 0);

	pifLog_Printf(LT_INFO, "Task=%d Timer=%d\n", pifTaskManager_Count(), pifTimerManager_Count(&g_timer_1ms));
}

