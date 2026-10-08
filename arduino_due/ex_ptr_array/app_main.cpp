#include "app_main.h"

#include "core/pif_ptr_array.h"


#define ARRAY_SIZE				10
#define TEST_COUNT				1000

// One step adds or deletes one node and prints one line of about 130 bytes, which a 115200bps
// UART sends in about 12ms. pifLog drops what does not fit in its transmit buffer, so the steps
// are spread out to let the buffer drain between them.
#define TEST_PERIOD				20000		// 20ms

#define NODE_INDEX(it)			((int)((it) - s_array._p_node))


static PifPtrArray s_array;
static int s_test = 0;
static int s_num = 0;
static BOOL s_delete;


static void list_print(const char *mode, int index)
{
	PifPtrArrayIterator it;

	pifLog_Printf(LT_NONE, "%s:%2d  %2d:%2d ", mode, index, s_array._p_free ? NODE_INDEX(s_array._p_free) : -1, s_array._p_first ? NODE_INDEX(s_array._p_first) : -1);

	for (int i = 0; i < s_array._max_count; i++) {
		it = &s_array._p_node[i];
		pifLog_Printf(LT_NONE, "%2d:%2d ", it->p_prev ? NODE_INDEX(it->p_prev) : -1, it->p_next ? NODE_INDEX(it->p_next) : -1);
	}

	pifLog_Printf(LT_NONE, "  C:%2d  U: ", s_array._count);
	it = s_array._p_first;
	if (it) {
		while (it) {
			pifLog_Printf(LT_NONE, "%2d ", NODE_INDEX(it));
			it = it->p_next;
		}
	}
	else {
		pifLog_Print(LT_NONE, "-1 ");
	}

	pifLog_Print(LT_NONE, " F: ");
	it = s_array._p_free;
	if (it) {
		while (it) {
			pifLog_Printf(LT_NONE, "%2d ", NODE_INDEX(it));
			it = it->p_next;
		}
	}
	else {
		pifLog_Print(LT_NONE, "-1 ");
	}
	pifLog_Print(LT_NONE, "\n");
}

static BOOL list_add()
{
	PifPtrArrayIterator it;

	it = pifPtrArray_Add(&s_array, NULL);
	if (!it) return FALSE;

	list_print("Add ", NODE_INDEX(it));
	return TRUE;
}

static void list_delete(int16_t index)
{
	PifPtrArrayIterator it;

	it = s_array._p_first;
	while (it) {
		if (NODE_INDEX(it) == index) break;
		it = it->p_next;
	}
	if (!it) return;

	pifPtrArray_Remove(&s_array, it);

	list_print("Del ", index);
}

static int list_find(int index)
{
	int i;
	PifPtrArrayIterator it = s_array._p_first;
	for (i = 0; i < index; i++) {
		it = it->p_next;
	}
	return NODE_INDEX(it);
}

// Each test either adds 1 to 6 nodes or deletes fewer nodes than are in use, one node per call.
static uint32_t _taskTest(PifTask *p_task)
{
	while (!s_num) {
		if (s_test >= TEST_COUNT) {
			pifLog_Printf(LT_NONE, "\nDone: test=%d, drop=%lu\n", s_test, (unsigned long)pifLog_DropCount());
			p_task->pause = TRUE;
			return 0;
		}

		s_delete = s_test & 1;
		if (s_delete) {
			s_num = rand() % s_array._count;
		}
		else {
			s_num = rand() % 6 + 1;
			if (s_num > ARRAY_SIZE - s_array._count) s_num = ARRAY_SIZE - s_array._count;
		}
		s_test++;
	}

	if (s_delete) list_delete(list_find(rand() % s_array._count));
	else list_add();
	s_num--;
	return 0;
}

BOOL appSetup()
{
	pifLog_Print(LT_NONE, "\n         F: U   0     1     2     3     4     5     6     7     8     9\n");

	if (!pifPtrArray_Init(&s_array, ARRAY_SIZE, NULL)) return FALSE;
	list_print("Init", 0);

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, TEST_PERIOD, _taskTest, NULL, TRUE)) return FALSE;
	return TRUE;
}
