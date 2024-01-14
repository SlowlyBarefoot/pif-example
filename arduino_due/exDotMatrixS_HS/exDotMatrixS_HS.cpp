// Do not remove the include below
#include "exDotMatrixS_HS.h"
#include "appMain.h"


#define PIN_LED_L				13

#define PIN_1					23
#define PIN_2					25
#define PIN_3					27
#define PIN_4					29
#define PIN_5					31
#define PIN_6					33
#define PIN_7					35
#define PIN_8					37
#define PIN_9					39
#define PIN_10					41
#define PIN_11					43
#define PIN_12					45
#define PIN_13					47
#define PIN_14					49
#define PIN_15					51
#define PIN_16					53

#define TASK_SIZE				5
#define TIMER_1MS_SIZE			2

#define UART_LOG_BAUDRATE		115200


const uint8_t c_ucPinDotMatrixCol[] = { PIN_13, PIN_3, PIN_4, PIN_10, PIN_6, PIN_11, PIN_15, PIN_16 };
const uint8_t c_ucPinDotMatrixRow[] = { PIN_9, PIN_14, PIN_8, PIN_12, PIN_1, PIN_7, PIN_2, PIN_5 };


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	(void)p_uart;

    return Serial.write((char *)pucBuffer, usSize);
}

static void actDotMatrixDisplay(uint8_t ucRow, uint8_t *pucData)
{
	static int row = -1;

	if (row >= 0) digitalWrite(c_ucPinDotMatrixRow[row], LOW);

	digitalWrite(c_ucPinDotMatrixRow[ucRow], HIGH);
	for (int col = 0; col < 8; col++) {
		digitalWrite(c_ucPinDotMatrixCol[col], !((*pucData >> col) & 1));
	}
	row = ucRow;
}

static uint16_t taskLedToggle(PifTask *pstTask)
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

	for (int i = 0; i < 8; i++) {
		pinMode(c_ucPinDotMatrixCol[i], OUTPUT);
		pinMode(c_ucPinDotMatrixRow[i], OUTPUT);
	}

	Serial.begin(UART_LOG_BAUDRATE); //Doesn't matter speed

    pif_Init(NULL);

    if (!pifTaskManager_Init(5)) return;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, 2)) return;					// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;					// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifDotMatrix_Init(&g_dot_matrix, PIF_ID_AUTO, &g_timer_1ms, 8, 8, actDotMatrixDisplay)) return;

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;			// 500ms

    if (!appSetup()) return;

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***         exDotMatrixS_HS          ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
