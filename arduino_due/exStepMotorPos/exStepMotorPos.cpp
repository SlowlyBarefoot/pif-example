// Do not remove the include below
#include <DueTimer.h>

#include "exStepMotorPos.h"
#include "appMain.h"


#define PIN_LED_L				13
#define PIN_STEP_MOTOR_1		30
#define PIN_STEP_MOTOR_2		32
#define PIN_STEP_MOTOR_3		34
#define PIN_STEP_MOTOR_4		36
#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

void taskTerminal(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(g_pstSerial, &txData)) {
    	SerialUSB.print((char)txData);
    }

    if (pifComm_GetRemainSizeOfRxBuffer(g_pstSerial)) {
		if (SerialUSB.available()) {
			rxData = SerialUSB.read();
			pifComm_ReceiveData(g_pstSerial, rxData);
		}
    }
}

uint16_t actPhotoInterruptAcquire(PIF_usId usPifId)
{
	switch (usPifId) {
	case PIF_ID_SWITCH:
		return digitalRead(PIN_PHOTO_INTERRUPT_1);

	case PIF_ID_SWITCH + 1:
		return digitalRead(PIN_PHOTO_INTERRUPT_2);

	case PIF_ID_SWITCH + 2:
		return digitalRead(PIN_PHOTO_INTERRUPT_3);
	}
	return OFF;
}

void actSetStep(uint16_t usPhase)
{
	digitalWrite(PIN_STEP_MOTOR_1, usPhase & 1);
	digitalWrite(PIN_STEP_MOTOR_2, (usPhase >> 1) & 1);
	digitalWrite(PIN_STEP_MOTOR_3, (usPhase >> 2) & 1);
	digitalWrite(PIN_STEP_MOTOR_4, (usPhase >> 3) & 1);
}

void actLedLState(PIF_usId usPifId, uint32_t unState)
{
	(void)usPifId;

	digitalWrite(PIN_LED_L, unState & 1);
}

extern "C" {
	void sysTickHook()
	{
		pif_sigTimer1ms();

		pifPulse_sigTick(g_pstTimer1ms);
	}
}

static void _sigTimer200us()
{
	pifPulse_sigTick(g_pstTimer200us);
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_STEP_MOTOR_1, OUTPUT);
	pinMode(PIN_STEP_MOTOR_2, OUTPUT);
	pinMode(PIN_STEP_MOTOR_3, OUTPUT);
	pinMode(PIN_STEP_MOTOR_4, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

	Timer3.attachInterrupt(_sigTimer200us).start(200);

	Serial.begin(115200);
	SerialUSB.begin(115200);

	appSetup();
}

// The loop function is called in an endless loop
void loop()
{
    pif_Loop();

    pifTask_Loop();
}
