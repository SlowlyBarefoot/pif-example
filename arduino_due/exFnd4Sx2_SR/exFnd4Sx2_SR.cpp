// Do not remove the include below
#include "exFnd4Sx2_SR.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_FND_COM1			0
#define PIN_FND_COM2			1
#define PIN_FND_COM3			2
#define PIN_FND_COM4			3
#define PIN_74HC595_SHIFT		4
#define PIN_74HC595_LATCH		5
#define PIN_74HC595_DATA		6

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			1

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

//The setup function is called once at startup of the sketch
void setup()
{
	static PifUart s_uart_log;

	pinMode(PIN_LED_L, OUTPUT);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 7; j++) {
			pinMode(c_unPin[i][j], OUTPUT);
		}
	}

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init(micros);

    if (!pifTaskManager_Init(TASK_SIZE)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return;	// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD, 1000, NULL)) return;				// 1ms
	s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifFnd_Init(&g_fnd, PIF_ID_AUTO, &g_timer_1ms, 8, actFndDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD, 500000, taskLedToggle, NULL, TRUE)) return;		// 500ms

	if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***           exFnd4Sx2_SR           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
