#include "ex_task_statistics.h"
#include "app_port.h"

#include <MsTimer2.h>


#define PIN_REALTIME			12

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	int room;

	(void)p_uart;

	// Serial.write() spins until its 64 byte buffer has room, which holds the CPU for as long as
	// the bytes take on the wire and delays the realtime task by milliseconds. Only what fits is
	// handed over: the log keeps the rest, and the UART task sends it once this part is out.
	room = Serial.availableForWrite();
	if (room <= 0) return 0;
	if (usSize > (uint16_t)room) usSize = room;
    return Serial.write((const char *)pucBuffer, usSize);
}

void actRealtimeSignal(BOOL state)
{
	digitalWrite(PIN_REALTIME, state);
}

static void sysTickHook()
{
	pif_sigTimer1ms();
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_REALTIME, OUTPUT);

	MsTimer2::set(1, sysTickHook);
	MsTimer2::start();

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

	// Nothing is read from the log, so it has no receiver task.
	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	// The banner and the CSV headers go out at once, about 180 bytes. A backup prints a line a step.
	if (!pifLog_AttachUart(&s_uart_log, 224)) return FALSE;										// 224bytes
	return TRUE;
}
