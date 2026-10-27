#include "ex_task_statistics.h"
#include "app_port.h"


#define PIN_REALTIME			12

#define UART_LOG_BAUDRATE		115200


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *pucBuffer, uint16_t usSize)
{
	int room;

	(void)p_uart;

	// Serial.write() spins until its 128 byte buffer has room, which holds the CPU for as long as
	// the bytes take on the wire and delays the realtime task by milliseconds. Only what fits is
	// handed over: the log keeps the rest, and the UART task sends it once this part is out.
	room = Serial.availableForWrite();
	if (room <= 0) return 0;
	if (usSize > room) usSize = room;
    return Serial.write((char *)pucBuffer, usSize);
}

static uint16_t actLogReceiveData(PifUart *p_uart, uint8_t *p_data, uint16_t size)
{
	int i, data;

	(void)p_uart;

	for (i = 0; i < size; i++) {
		data = Serial.read();
		if (data < 0) break;
		p_data[i] = data;
	}
	return i;
}

void actRealtimeSignal(BOOL state)
{
	digitalWrite(PIN_REALTIME, state);
}

extern "C" {
	int sysTickHook()
	{
		pif_sigTimer1ms();
		return 0;
	}
}

BOOL portSetup()
{
	static PifUart s_uart_log;

	pinMode(PIN_REALTIME, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 0)) return FALSE;

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, NULL)) return FALSE;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 50000, NULL)) return FALSE;	// 50ms
    s_uart_log.act_receive_data = actLogReceiveData;
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	// The table of the task command goes out at once, more than the 256 bytes the other examples keep.
	if (!pifLog_AttachUart(&s_uart_log, 1024)) return FALSE;										// 1024bytes
	return TRUE;
}
