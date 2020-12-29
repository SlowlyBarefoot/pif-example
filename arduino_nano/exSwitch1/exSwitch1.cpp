// Do not remove the include below
#include "exSwitch1.h"

#include "pifLog.h"
#include "pifSwitch.h"
#include "pifTask.h"


#define PIN_LED_RED				2
#define PIN_LED_YELLOW			3
#define PIN_PUSH_SWITCH			5
#define PIN_TILT_SWITCH			6

#define TASK_COUNT              1
#define SWITCH_COUNT            2


static PIF_stSwitch *s_pstPushSwitch = NULL;
static PIF_stSwitch *s_pstTiltSwitch = NULL;


static void _LogPrint(char *pcString)
{
	Serial.print(pcString);
}

static SWITCH _PushSwitchAcquire(PIF_unDeviceCode unDeviceCode)
{
	(void)unDeviceCode;

	return digitalRead(PIN_PUSH_SWITCH);
}

static void _PushSwitchChange(PIF_unDeviceCode unDeviceCode, SWITCH swState)
{
	(void)unDeviceCode;

	digitalWrite(PIN_LED_RED, swState);
}

static SWITCH _TiltSwitchAcquire(PIF_unDeviceCode unDeviceCode)
{
	(void)unDeviceCode;

	return digitalRead(PIN_TILT_SWITCH);
}

static void _TiltSwitchChange(PIF_unDeviceCode unDeviceCode, SWITCH swState)
{
	(void)unDeviceCode;

	digitalWrite(PIN_LED_YELLOW, swState);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_YELLOW, OUTPUT);
	pinMode(PIN_PUSH_SWITCH, INPUT_PULLUP);
	pinMode(PIN_TILT_SWITCH, INPUT_PULLUP);

	Serial.begin(115200); //Doesn't matter speed

    pif_Init();

    pifLog_Init();
	pifLog_AttachActPrint(_LogPrint);

    if (!pifSwitch_Init(SWITCH_COUNT)) return;

    s_pstPushSwitch = pifSwitch_Add(1, 0);
    if (!s_pstPushSwitch) return;
    s_pstPushSwitch->bStateReverse = TRUE;
    s_pstPushSwitch->actAcquire = _PushSwitchAcquire;
    s_pstPushSwitch->evtChange = _PushSwitchChange;

    s_pstTiltSwitch = pifSwitch_Add(2, 0);
	if (!s_pstTiltSwitch) return;
	s_pstTiltSwitch->actAcquire = _TiltSwitchAcquire;
	s_pstTiltSwitch->evtChange = _TiltSwitchChange;

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(3, pifSwitch_taskAll, NULL)) return;		// 3%
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
