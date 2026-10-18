#include "exFnd4Sx2_SR.h"
#include "app_port.h"


#define PIN_LED_L				13

#define PIN_FND_COM1			0
#define PIN_FND_COM2			1
#define PIN_FND_COM3			2
#define PIN_FND_COM4			3
#define PIN_74HC595_SHIFT		4
#define PIN_74HC595_LATCH		5
#define PIN_74HC595_DATA		6

#define UART_LOG_BAUDRATE		115200


const uint8_t c_unPin[2][7] = {
		{			// FND 1
			41,		// COM1
			43,		// COM2
			45,		// COM3
			47,		// COM4
			49,		// 74HC595 Shift
			51,		// 74HC595 Latch
			53		// 74HC595 Data
		},
		{			// FND 2
			27,		// COM1
			29,		// COM2
			31,		// COM3
			33,		// COM4
			35,		// 74HC595 Shift
			37,		// 74HC595 Latch
			39		// 74HC595 Data
		}
};


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actFndDisplay(uint8_t ucSegment, uint8_t ucDigit)
{
	int block = ucDigit >> 2;

	for (int j = 0; j < 4; j++) {
		digitalWrite(c_unPin[0][j], j != ucDigit);
		digitalWrite(c_unPin[1][j], j + 4 != ucDigit);
	}

	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], LOW);
	shiftOut(c_unPin[block][PIN_74HC595_DATA], c_unPin[block][PIN_74HC595_SHIFT], MSBFIRST, ucSegment);
	digitalWrite(c_unPin[block][PIN_74HC595_LATCH], HIGH);
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

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 7; j++) {
			pinMode(c_unPin[i][j], OUTPUT);
		}
	}

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init((PifActTimer1us)micros);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;			// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 256)) return FALSE;												// 256bytes

    if (!pifFnd_Init(&g_fnd, PIF_ID_AUTO, 8, actFndDisplay)) return FALSE;

    if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return FALSE;	// 500ms
	return TRUE;
}
