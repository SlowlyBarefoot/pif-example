// Do not remove the include below
#include <MsTimer2.h>

#include "exServoMotor.h"

#include "pifLog.h"
#include "pifPulse.h"


#define PIN_LED_L				13
#define PIN_PWM					9

#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	1
#define TASK_COUNT              3


static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stPulseItem *s_pstPwm = NULL;

static BOOL s_bStart = FALSE;


static void _actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

static void _actPulsePwm(SWITCH swValue)
{
	digitalWrite(PIN_PWM, swValue);
}

static void _taskServoMotor(PIF_stTask *pstTask)
{
	static float duty = 1.5 / 20;
	static BOOL dir = FALSE;

	(void)pstTask;

	if (s_pstPwm->_enStep == PS_enStop) return;

    if (dir) {
        duty -= 0.005;
        if (duty <= 0.5 / 20) {
        	dir = FALSE;
        }
    }
    else {
        duty += 0.005;
        if (duty >= 1.5 / 20) {
        	dir = TRUE;
        }
    }
    pifPulse_SetPwmDuty(s_pstPwm, PIF_PWM_MAX_DUTY * duty);
}

static void _taskLedToggle(PIF_stTask *pstTask)
{
	static BOOL sw = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, sw);
	sw ^= 1;

	if (!s_bStart) {
		if (pif_stPerformance._bReady) {
		    pifPulse_StartItem(s_pstPwm, 20);								// 20ms
		    pifPulse_SetPwmDuty(s_pstPwm, PIF_PWM_MAX_DUTY * 1.0 / 20);		// 1.0ms / 20ms = 7.5%
		    s_bStart = TRUE;
		}
	}
}

static void sysTickHook()
{
	pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer1ms);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_PWM, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(115200);

	pif_Init();

	pifLog_Init();
	pifLog_AttachActPrint(_actLogPrint);

    if (!pifPulse_Init(PULSE_COUNT)) return;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) return;

    s_pstPwm = pifPulse_AddItem(s_pstTimer1ms, PT_enPwm);
    if (!s_pstPwm) return;
    pifPulse_AttachAction(s_pstPwm, _actPulsePwm);

    if (!pifTask_Init(TASK_COUNT)) return;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) return;		// 100%
    if (!pifTask_AddPeriodMs(700, _taskServoMotor, NULL)) return;	// 1000ms
    if (!pifTask_AddPeriodUs(500, _taskLedToggle, NULL)) return;	// 500ms
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
