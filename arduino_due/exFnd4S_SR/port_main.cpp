#include "exFnd4S_SR.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_74HC595_DATA		46
#define PIN_74HC595_LATCH		44
#define PIN_74HC595_SHIFT		42

#define UART_LOG_BAUDRATE		115200


const uint8_t c_unPinCom[] = {
		40,		// COM1
		38,		// COM2
		36,		// COM3
		34		// COM4
};


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPinCom[j], j != ucDigit);
	}

	digitalWrite(PIN_74HC595_LATCH, LOW);
	shiftOut(PIN_74HC595_DATA, PIN_74HC595_SHIFT, MSBFIRST, ucSegment);
	digitalWrite(PIN_74HC595_LATCH, HIGH);
}

static uint32_t taskLedToggle(PifTask *pstTask)
{
	static BOOL swLed = LOW;

	(void)pstTask;

	digitalWrite(PIN_LED_L, swLed);
	swLed ^= 1;
	return 0;
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		pifTimerManager_sigTick(&g_timer_1ms);
		return 0;
	}
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	pinMode(PIN_74HC595_DATA, OUTPUT);
	pinMode(PIN_74HC595_LATCH, OUTPUT);
	pinMode(PIN_74HC595_SHIFT, OUTPUT);
	for (int i = 0; i < 4; i++) {
		pinMode(c_unPinCom[i], OUTPUT);
	}

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) goto fail;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) goto fail;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) goto fail;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) goto fail;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) goto fail;												// 256bytes

    if (!pifFnd_Init(&g_fnd, PIF_ID_AUTO, 4, actFndDisplay)) goto fail;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) goto fail;	// 500ms
	return TRUE;

fail:
	pif_Exit();
	return FALSE;
}
