#include "ex_can_loop.h"
#include "app_port.h"

#include <due_can.h>

// Needs the due_can and can_common libraries, added to the project from the Arduino library manager.
//
// CAN0 and CAN1 each need a 3.3V CAN transceiver, such as an SN65HVD230:
//   CAN0: CANTX -> TXD, CANRX <- RXD of transceiver 0
//   CAN1: D53 (CANTX1) -> TXD, DAC0 (CANRX1) <- RXD of transceiver 1
// Join CANH to CANH and CANL to CANL, with a 120 ohm resistor at each end of the bus.
// CAN0 sends a frame every second and CAN1 sends it back. With PIF_USE_COMM_TAP the taps of both print every
// frame they receive and send.


#define PIN_LED_L				13

#define UART_LOG_BAUDRATE		115200

#define CAN_BITRATE				500000
#define CAN_RX_COUNT			16		// Frames each receive queue holds.
#define CAN_TX_COUNT			8		// Frames each send queue holds.


PifCan g_can0;
PifCan g_can1;


static uint16_t actLogSendData(PifUart *p_uart, uint8_t *p_buffer, uint16_t size)
{
	(void)p_uart;

    return Serial.write((char *)p_buffer, size);
}

static void actLedLState(PifId pid_id, uint32_t state)
{
	(void)pid_id;

	digitalWrite(PIN_LED_L, state & 1);
}

static uint32_t actIrqDisable()
{
	uint32_t primask = __get_PRIMASK();

	__disable_irq();
	return primask;
}

static void actIrqRestore(uint32_t state)
{
	__set_PRIMASK(state);
}

static CANRaw &getCanPort(PifCan *p_can)
{
	return p_can == &g_can0 ? Can0 : Can1;
}

static BOOL actCanSendFrame(PifCan *p_can, const PifCanFrame *p_frame)
{
	CAN_FRAME frame;

	frame.id = p_frame->id;
	frame.extended = (p_frame->flags & CAN_F_EXTENDED) != 0;
	frame.rtr = (p_frame->flags & CAN_F_RTR) != 0;
	frame.length = p_frame->length;
	memcpy(frame.data.bytes, p_frame->data, p_frame->length);

	// FALSE while due_can cannot take the frame: PifCan keeps it and tries again.
	return getCanPort(p_can).sendFrame(frame);
}

// due_can calls the receive events from its interrupt, where pifCan_PutRxFrame() may be called.
static void putCanRxFrame(PifCan *p_can, CAN_FRAME *p_frame)
{
	PifCanFrame frame;

	frame.id = p_frame->id;
	frame.flags = (p_frame->extended ? CAN_F_EXTENDED : CAN_F_NONE) | (p_frame->rtr ? CAN_F_RTR : CAN_F_NONE);
	frame.length = p_frame->length > 8 ? 8 : p_frame->length;
	memcpy(frame.data, p_frame->data.bytes, frame.length);
	pifCan_PutRxFrame(p_can, &frame);
}

static void evtCan0Receive(CAN_FRAME *p_frame)
{
	putCanRxFrame(&g_can0, p_frame);
}

static void evtCan1Receive(CAN_FRAME *p_frame)
{
	putCanRxFrame(&g_can1, p_frame);
}

static PifCanBusState getCanBusState(CANRaw &can)
{
	uint32_t status = can.get_status();

	if (status & CAN_SR_BOFF) return CBS_BUS_OFF;
	if (status & CAN_SR_ERRP) return CBS_ERROR_PASSIVE;
	return CBS_ERROR_ACTIVE;
}

static void evtCanBusState(PifCan *p_can, PifCanBusState state)
{
	static const char *c_state[] = { "Error active", "Error passive", "Bus off" };

	pifLog_Printf(LT_WARN, "CAN%d: %s", p_can == &g_can0 ? 0 : 1, c_state[state]);
}

static uint32_t taskCanBusState(PifTask *p_task)
{
	(void)p_task;

	// due_can reports the error state only in the status register, so it is polled.
	pifCan_SigBusState(&g_can0, getCanBusState(Can0));
	pifCan_SigBusState(&g_can1, getCanBusState(Can1));
	return 0;
}

static BOOL setupCan(PifCan *p_can, void (*evt_receive)(CAN_FRAME *), const char *p_rx_name, const char *p_tx_name)
{
	CANRaw &can = getCanPort(p_can);

	if (!pifCan_Init(p_can, PIF_ID_AUTO, CAN_BITRATE, 0, CAN_RX_COUNT, CAN_TX_COUNT)) return FALSE;
	// Received frames trigger the RX task. The TX task runs every 1ms to hand queued frames to due_can.
	if (!pifCan_AttachRxTask(p_can, PIF_ID_AUTO, TM_EXTERNAL, 0, p_rx_name)) return FALSE;
	if (!pifCan_AttachTxTask(p_can, PIF_ID_AUTO, TM_PERIOD, 1000, p_tx_name)) return FALSE;		// 1ms
	p_can->act_send_frame = actCanSendFrame;
	p_can->evt_bus_state = evtCanBusState;

	if (!can.begin(CAN_BITRATE)) return FALSE;
	can.watchFor();						// Every ID.
	can.setGeneralCallback(evt_receive);
	return TRUE;
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

	pif_act_irq_disable = actIrqDisable;
	pif_act_irq_restore = actIrqRestore;

	pinMode(PIN_LED_L, OUTPUT);

	Serial.begin(UART_LOG_BAUDRATE);

    if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

    if (!pifTimerManager_Init(&g_timer_1ms, PIF_ID_AUTO, 1000, TIMER_1MS_SIZE)) return FALSE;				// 1000us

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO, UART_LOG_BAUDRATE)) return FALSE;
    if (!pifUart_AttachTxTask(&s_uart_log, PIF_ID_AUTO, TM_EXTERNAL, 0, "UartTxLog")) return FALSE;
    if (!pifUart_AttachRxTask(&s_uart_log, PIF_ID_AUTO, TM_PERIOD, 1000, "UartRxLog")) return FALSE;				// 1ms
    s_uart_log.act_send_data = actLogSendData;

    pifLog_Init();
	if (!pifLog_AttachUart(&s_uart_log, 512)) return FALSE;												// 512bytes

    if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;

	if (!setupCan(&g_can0, evtCan0Receive, "CanRx0", "CanTx0")) return FALSE;
	if (!setupCan(&g_can1, evtCan1Receive, "CanRx1", "CanTx1")) return FALSE;
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 10000, taskCanBusState, NULL, TRUE)) return FALSE;				// 10ms
	return TRUE;
}
