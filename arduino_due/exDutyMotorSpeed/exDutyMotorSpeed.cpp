// Do not remove the include below
#include "exDutyMotorSpeed.h"
#include "appMain.h"

#include "pifLog.h"


#define PIN_LED_L				13
#define PIN_L298N_ENB_PWM		2
#define PIN_L298N_IN1			30
#define PIN_L298N_IN2			32
#define PIN_PHOTO_INTERRUPT_1	35
#define PIN_PHOTO_INTERRUPT_2	37
#define PIN_PHOTO_INTERRUPT_3	39


void actLogPrint(char *pcString)
{
	Serial.print(pcString);
}

uint16_t taskTerminal(PIF_stTask *pstTask)
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
    return 0;
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

void actSetDuty(uint16_t usDuty)
{
	analogWrite(PIN_L298N_ENB_PWM, usDuty);
	pifLog_Printf(LT_enInfo, "SetDuty(%d)", usDuty);
}

void actSetDirection(uint8_t ucDir)
{
	if (ucDir) {
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, HIGH);
	}
	else {
		digitalWrite(PIN_L298N_IN1, HIGH);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	pifLog_Printf(LT_enInfo, "SetDirection(%d)", ucDir);
}

void actOperateBreak(uint8_t ucState)
{
	if (ucState) {
		analogWrite(PIN_L298N_ENB_PWM, 255);
		digitalWrite(PIN_L298N_IN1, LOW);
		digitalWrite(PIN_L298N_IN2, LOW);
	}
	else {
		analogWrite(PIN_L298N_ENB_PWM, 0);
	}
	pifLog_Printf(LT_enInfo, "OperateBreak(%d)", ucState);
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

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(PIN_LED_L, OUTPUT);
	pinMode(PIN_L298N_ENB_PWM, OUTPUT);
	pinMode(PIN_L298N_IN1, OUTPUT);
	pinMode(PIN_L298N_IN2, OUTPUT);
	pinMode(PIN_PHOTO_INTERRUPT_1, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_2, INPUT_PULLUP);
	pinMode(PIN_PHOTO_INTERRUPT_3, INPUT_PULLUP);

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
