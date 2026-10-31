#include "app_port.h"

#include <string.h>


#define TAP_QUEUE_SIZE			16


PifLed g_led_l;
PifTimerManager g_timer_1ms;

static PifCanFrame s_sent;
static uint8_t s_count = 0;

#ifdef PIF_USE_COMM_TAP

// Frames the taps of CAN0 and CAN1 caught, for the print task.
typedef struct {
	uint8_t can;
	uint8_t dir;			// PifTapDir
	PifCanFrame frame;
} TapEntry;

static TapEntry s_tap_queue[TAP_QUEUE_SIZE];
static volatile uint8_t s_tap_head = 0, s_tap_tail = 0;
static volatile uint32_t s_tap_overflow = 0;

#endif


static void _printFrame(const char *p_name, const PifCanFrame *p_frame)
{
	pifLog_Printf(LT_INFO, "%s: ID=%3X LEN=%u", p_name, p_frame->id, p_frame->length);
	if (p_frame->length) {
		pifLog_Print(LT_NONE, " DATA=");
		for (int i = 0; i < p_frame->length; i++) pifLog_Printf(LT_NONE, " %2X", p_frame->data[i]);
	}
}

// CAN1 sends back what it got from CAN0.
static void _evtCan1Receive(PifCan *p_can, const PifCanFrame *p_frame)
{
	PifCanFrame frame = *p_frame;

	if (p_frame->id != CAN_ID_0_TO_1) return;
	frame.id = CAN_ID_1_TO_0;
	if (!pifCan_SendFrame(p_can, &frame)) {
		pifLog_Printf(LT_ERROR, "CAN1: Send E=%d", pif_error);
	}
}

// CAN0 compares the frame that came back with the one it sent.
static void _evtCan0Receive(PifCan *p_can, const PifCanFrame *p_frame)
{
	(void)p_can;

	if (p_frame->id != CAN_ID_1_TO_0) return;
	if (p_frame->length != s_sent.length || memcmp(p_frame->data, s_sent.data, p_frame->length)) {
		_printFrame("CAN0: Different", p_frame);
	}
	else {
		pifLog_Printf(LT_INFO, "CAN0: Loop OK LEN=%u", p_frame->length);
	}
}

static uint32_t _taskSend(PifTask *p_task)
{
	(void)p_task;

	s_sent.id = CAN_ID_0_TO_1;
	s_sent.flags = CAN_F_NONE;
	s_sent.length = s_count % 9;
	for (int i = 0; i < s_sent.length; i++) s_sent.data[i] = s_count + i;
	s_count++;

	if (!pifCan_SendFrame(&g_can0, &s_sent)) {
		pifLog_Printf(LT_ERROR, "CAN0: Send E=%d", pif_error);
	}
	return 0;
}

#ifdef PIF_USE_COMM_TAP

// Called from the receive interrupt of due_can for TD_RX and TD_RX_LOST, and from the TX task for TD_TX,
// so it only queues the frame with the interrupts held off by pif_IrqDisable().
static void _evtCanTap(PifCan *p_can, PifTapDir dir, const PifCanFrame *p_frame)
{
	uint32_t state = pif_IrqDisable();
	uint8_t next;

	next = (s_tap_head + 1) % TAP_QUEUE_SIZE;
	if (next == s_tap_tail) {
		s_tap_overflow++;
	}
	else {
		s_tap_queue[s_tap_head].can = p_can == &g_can0 ? 0 : 1;
		s_tap_queue[s_tap_head].dir = dir;
		s_tap_queue[s_tap_head].frame = *p_frame;
		s_tap_head = next;
	}
	pif_IrqRestore(state);
}

static uint32_t _taskPrintTap(PifTask *p_task)
{
	static const char *c_dir[] = { "Rx", "Tx", "RxLost" };
	char name[16];

	(void)p_task;

	while (s_tap_tail != s_tap_head) {
		TapEntry *p_entry = &s_tap_queue[s_tap_tail];

		pif_Printf(name, sizeof(name), "Tap%d %s", p_entry->can, c_dir[p_entry->dir]);
		_printFrame(name, &p_entry->frame);
		s_tap_tail = (s_tap_tail + 1) % TAP_QUEUE_SIZE;
	}
	if (s_tap_overflow) {
		pifLog_Printf(LT_WARN, "Tap: %lu frames not printed", (unsigned long)s_tap_overflow);
		s_tap_overflow = 0;
	}
	return 0;
}

#endif

BOOL appSetup()
{
	g_can0.evt_receive = _evtCan0Receive;
	g_can1.evt_receive = _evtCan1Receive;

#ifdef PIF_USE_COMM_TAP
	g_can0.evt_tap = _evtCanTap;
	g_can1.evt_tap = _evtCanTap;
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 10000, _taskPrintTap, NULL, TRUE)) return FALSE;	// 10ms
#endif

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000000, _taskSend, NULL, TRUE)) return FALSE;		// 1s

	if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
	g_led_l._p_task->name = "LedL";
	pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifLog_Print(LT_NONE, "\n\n****************************************\n");
	pifLog_Print(LT_NONE, "***            ex_can_loop           ***\n");
	pifLog_Printf(LT_NONE, "***       %s %s       ***\n", __DATE__, __TIME__);
	pifLog_Print(LT_NONE, "****************************************\n");
	pifLog_Printf(LT_INFO, "Task=%d/%d Timer=%d/%d\n", pifTaskManager_Count(), TASK_SIZE, pifTimerManager_Count(&g_timer_1ms), TIMER_1MS_SIZE);
	return TRUE;
}
