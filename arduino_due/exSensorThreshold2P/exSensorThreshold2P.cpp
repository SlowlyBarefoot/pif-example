// Do not remove the include below
#include "exSensorThreshold2P.h"

#include "pifLog.h"
#include "pifPulse.h"
#include "pifSensor.h"
#include "pifTask.h"


#define PIN_LED_L				13
#define PIN_CDS					A0

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	5
#define SENSOR_COUNT         	1
#define TASK_COUNT              4

#define USE_FILTER_AVERAGE		0


static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stSensor *s_pstSensor = NULL;
#if USE_FILTER_AVERAGE
static PIF_stSensorFilter s_stFilter;
#endif


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _LedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;
}

static void _SensorAcquisition(PIF_stTask *pstTask)
{
	(void)pstTask;

	pifSensor_sigData(s_pstSensor, analogRead(PIN_CDS));
}

static void _SensorThreshold(PIF_usId usPifId, SWITCH swState)
{
	pifLog_Printf(LT_enInfo, "Sensor: DC:%u SW:%u", usPifId, swState);
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(s_pstTimer1ms);
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_CDS, INPUT);

	Serial.begin(115200); //Doesn't matter speed

	pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT);
    if (!s_pstTimer1ms) return;

    if (!pifSensor_Init(s_pstTimer1ms, SENSOR_COUNT)) return;
    s_pstSensor = pifSensor_Add(PIF_ID_AUTO);
    if (!s_pstSensor) return;
#if USE_FILTER_AVERAGE
    pifSensor_AttachFilter(s_pstSensor, PIF_SENSOR_FILTER_AVERAGE, 7, &s_stFilter, TRUE);
#endif
    pifSensor_SetEventThreshold2P(s_pstSensor, 400, 700, _SensorThreshold);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;			// 100%
    if (!pifTask_AddPeriodMs(100, pifSensor_taskAll, NULL)) return;		// 100ms

    if (!pifTask_AddPeriodMs(500, _LedToggle, NULL)) return;			// 500ms
    if (!pifTask_AddPeriodMs(100, _SensorAcquisition, NULL)) return;	// 100ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
