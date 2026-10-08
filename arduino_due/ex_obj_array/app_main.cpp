#include "app_main.h"

#include "core/pif_obj_array.h"


#define ARRAY_SIZE				10
#define TEST_COUNT				1000

// One step adds or deletes one node and prints one line of about 130 bytes, which a 115200bps
// UART sends in about 12ms. pifLog drops what does not fit in its transmit buffer, so the steps
// are spread out to let the buffer drain between them.
#define TEST_PERIOD				20000		// 20ms

#define NODE_SIZE				(2 * sizeof(PifObjArrayIterator) + s_array._size)


static PifObjArray s_array;
static int s_test = 0;
static int s_num = 0;
static BOOL s_delete;


static void list_print(const char *mode, int index)
{
	char *p_buffer = (char *)s_array._p_node;
	PifObjArrayIterator it;

	pifLog_Printf(LT_NONE, "%s:%2d  %2d:%2d ", mode, index, s_array._p_free ? s_array._p_free->data[0] : -1, s_array._p_first ? s_array._p_first->data[0] : -1);

	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		pifLog_Printf(LT_NONE, "%2d:%2d ", it->p_prev ? it->p_prev->data[0] : -1, it->p_next ? it->p_next->data[0] : -1);
		p_buffer += NODE_SIZE;
	}

	pifLog_Printf(LT_NONE, "  C:%2d  U: ", s_array._count);
	it = s_array._p_first;
	if (it) {
		while (it) {
			pifLog_Printf(LT_NONE, "%2d ", it->data[0]);
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
			pifLog_Printf(LT_NONE, "%2d ", it->data[0]);
			it = it->p_next;
		}
	}
	else {
		pifLog_Print(LT_NONE, "-1 ");
	}
	pifLog_Print(LT_NONE, "\n");
}

static char *list_add()
{
	PifObjArrayIterator it;

	it = pifObjArray_Add(&s_array);
	if (!it) return NULL;

	// pifObjArray_Add() clears the data, so the node index is written again.
	it->data[0] = ((char *)it - (char *)s_array._p_node) / NODE_SIZE;

	list_print("Add ", it->data[0]);
	return (char *)it->data;
}

static void list_delete(int16_t index)
{
	PifObjArrayIterator it;

	it = s_array._p_first;
	while (it) {
		if (it->data[0] == index) break;
		it = it->p_next;
	}
	if (!it) return;

	pifObjArray_Remove(&s_array, it->data);

	list_print("Del ", index);
}

static int list_find(int index)
{
	int i;
	PifObjArrayIterator it = s_array._p_first;
	for (i = 0; i < index; i++) {
		it = it->p_next;
	}
	return it->data[0];
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
	char *p_buffer;
	PifObjArrayIterator it;

	pifLog_Print(LT_NONE, "\n         F: U   0     1     2     3     4     5     6     7     8     9\n");

	if (!pifObjArray_Init(&s_array, sizeof(char), ARRAY_SIZE, NULL)) return FALSE;
	p_buffer = (char *)s_array._p_node;
	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		it->data[0] = i;
		p_buffer += NODE_SIZE;
	}
	list_print("Init", 0);

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, TEST_PERIOD, _taskTest, NULL, TRUE)) return FALSE;
	return TRUE;
}
