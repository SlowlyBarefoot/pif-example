#include "app_main.h"

#include "core/pif_bst.h"


#define KEY_RANGE				20
#define TEST_COUNT				300

// One step prints at most about 300 bytes, which a 115200bps UART sends in about 26ms.
// pifLog drops what does not fit in its transmit buffer, so the steps are spread out
// to let the buffer drain between them.
#define TEST_PERIOD				50000		// 50ms


typedef struct
{
	uint16_t value;
	uint16_t count;
} ExData;


static PifBst s_bst;
static int s_error = 0;
static int s_clear_count;
static int s_step = 0;


static void tree_print_shape(PifBstIterator it)
{
	if (!it) {
		pifLog_Print(LT_NONE, "-");
		return;
	}

	pifLog_Printf(LT_NONE, "%d", (int)it->key);
	if (it->p_small || it->p_big) {
		pifLog_Print(LT_NONE, "(");
		tree_print_shape(it->p_small);
		pifLog_Print(LT_NONE, ",");
		tree_print_shape(it->p_big);
		pifLog_Print(LT_NONE, ")");
	}
}

// Returns the subtree height, or -1 when a link, key order, height or AVL balance is broken.
static int tree_check_node(PifBstIterator it, PifBstIterator p_parent, long min, long max)
{
	int small, big;

	if (!it) return 0;
	if (it->p_parent != p_parent) return -1;
	if ((long)it->key <= min || (long)it->key >= max) return -1;

	small = tree_check_node(it->p_small, it, min, it->key);
	if (small < 0) return -1;
	big = tree_check_node(it->p_big, it, it->key, max);
	if (big < 0) return -1;

	if (small - big > 1 || big - small > 1) return -1;
	if (it->height != 1 + MAX(small, big)) return -1;
	return 1 + MAX(small, big);
}

static BOOL tree_check()
{
	PifBstIterator it;
	int count = 0;

	if (tree_check_node(s_bst.p_root, NULL, -1, 0x7FFFFFFFL) < 0) return FALSE;

	for (it = pifBst_Begin(&s_bst); it; it = pifBst_Next(it)) count++;
	return count == pifBst_Size(&s_bst);
}

static void tree_print(const char *mode, int key)
{
	PifBstIterator it;
	BOOL ok = tree_check();

	if (!ok) s_error++;

	pifLog_Printf(LT_NONE, "%s:%2d  S:%2d  H:%d  %s  L: ", mode, key, pifBst_Size(&s_bst),
			s_bst.p_root ? s_bst.p_root->height : 0, ok ? "OK " : "ERR");

	it = pifBst_Begin(&s_bst);
	if (it) {
		while (it) {
			pifLog_Printf(LT_NONE, "%2d ", (int)it->key);
			it = pifBst_Next(it);
		}
	}
	else {
		pifLog_Print(LT_NONE, "-1 ");
	}

	pifLog_Print(LT_NONE, " T: ");
	tree_print_shape(s_bst.p_root);
	pifLog_Print(LT_NONE, "\n");
}

static void tree_print_data(const char *mode)
{
	PifBstIterator it;
	ExData *p_data;

	pifLog_Print(LT_NONE, mode);
	for (it = pifBst_Begin(&s_bst); it; it = pifBst_Next(it)) {
		p_data = (ExData *)it->data;
		pifLog_Printf(LT_NONE, " %d=%d/%d", (int)it->key, p_data->value, p_data->count);
	}
	pifLog_Print(LT_NONE, "\n");
}

static void evt_clear(PifBstKey key, void *p_data)
{
	(void)key;
	(void)p_data;

	s_clear_count++;
}

static BOOL test_random_step()
{
	int key;
	ExData *p_data;

	key = rand() % KEY_RANGE;
	if (pifBst_Find(&s_bst, key)) {
		pifBst_Remove(&s_bst, key);
		tree_print("Del ", key);
	}
	else {
		p_data = (ExData *)pifBst_Add(&s_bst, key);
		if (!p_data) {
			pifLog_Print(LT_NONE, "Add failed\n");
			s_error++;
			return FALSE;
		}
		p_data->value = key * 10;
		tree_print("Add ", key);
	}
	return TRUE;
}

static void test_api_set()
{
	int key;
	ExData data, *p_data;

	pifLog_Print(LT_NONE, "\n--- API ---\n");

	pifBst_Clear(&s_bst, NULL);
	for (key = 2; key <= 18; key += 2) {
		data.value = key * 10;
		data.count = 0;
		pifBst_Set(&s_bst, key, &data);
	}
	tree_print("Set ", 18);

	// An existing key is not added again.
	p_data = (ExData *)pifBst_Add(&s_bst, 10);
	pifLog_Printf(LT_NONE, "Add(10) again: %s, E_ALREADY_ATTACHED=%s\n", p_data ? "added" : "NULL",
			pif_error == E_ALREADY_ATTACHED ? "yes" : "no");

	// pifBst_Set with NULL works as get-or-add.
	for (key = 0; key < 30; key++) {
		p_data = (ExData *)pifBst_Set(&s_bst, rand() % 10 * 2, NULL);
		p_data->count++;
	}
	data.value = 999;
	data.count = 0;
	pifBst_Set(&s_bst, 10, &data);
	tree_print_data("Data:");

	p_data = (ExData *)pifBst_FindData(&s_bst, 10);
	pifLog_Printf(LT_NONE, "FindData(10)=%d  FindData(11)=%s\n", p_data ? p_data->value : -1,
			pifBst_FindData(&s_bst, 11) ? "found" : "NULL");
}

static void test_api_iterate()
{
	int key;
	PifBstIterator it;

	pifLog_Print(LT_NONE, "LowerBound:");
	for (key = 0; key <= 20; key += 5) {
		it = pifBst_LowerBound(&s_bst, key);
		pifLog_Printf(LT_NONE, " %d->%d", key, it ? (int)it->key : -1);
	}
	pifLog_Print(LT_NONE, "\n");

	pifLog_Print(LT_NONE, "Range [5, 13]:");
	for (it = pifBst_LowerBound(&s_bst, 5); it && it->key <= 13; it = pifBst_Next(it)) {
		pifLog_Printf(LT_NONE, " %d", (int)it->key);
	}
	pifLog_Print(LT_NONE, "\n");

	pifLog_Print(LT_NONE, "Reverse:");
	for (it = pifBst_End(&s_bst); it; it = pifBst_Prev(it)) {
		pifLog_Printf(LT_NONE, " %d", (int)it->key);
	}
	pifLog_Print(LT_NONE, "\n");

	// pifBst_Erase returns the next iterator, so nodes can be removed while walking.
	it = pifBst_Begin(&s_bst);
	while (it) {
		if (it->key % 4 == 0) it = pifBst_Erase(&s_bst, it);
		else it = pifBst_Next(it);
	}
	tree_print("Ers4", 0);

	s_clear_count = 0;
	pifBst_Clear(&s_bst, evt_clear);
	pifLog_Printf(LT_NONE, "Clear: callback=%d\n", s_clear_count);
	tree_print("Clr ", 0);
}

static uint32_t _taskTest(PifTask *p_task)
{
	if (s_step == 0) pifLog_Print(LT_NONE, "\n--- Random add/remove ---\n");

	if (s_step < TEST_COUNT) {
		if (!test_random_step()) s_step = TEST_COUNT - 1;
	}
	else if (s_step == TEST_COUNT) {
		test_api_set();
	}
	else if (s_step == TEST_COUNT + 1) {
		test_api_iterate();
	}
	else {
		pifLog_Printf(LT_NONE, "\nResult: %s (error=%d, drop=%lu)\n", s_error ? "FAIL" : "PASS", s_error,
				(unsigned long)pifLog_DropCount());
		p_task->pause = TRUE;
	}
	s_step++;
	return 0;
}

BOOL appSetup()
{
    pifLog_Print(LT_NONE, "S: size  H: root height  L: keys in order  T: key(small,big)\n");

	if (!pifBst_Init(&s_bst, sizeof(ExData))) return FALSE;

	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, TEST_PERIOD, _taskTest, NULL, TRUE)) return FALSE;
    return TRUE;
}
