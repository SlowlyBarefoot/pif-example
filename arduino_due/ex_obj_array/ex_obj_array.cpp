// Do not remove the include below
#include "ex_obj_array.h"

#include "core/pif_obj_array.h"


#define UART_LOG_BAUDRATE		115200

typedef struct {
	uint8_t data[8];
} TData;

static PifObjArray s_array;


void print()
{
	char str[10];
	char *p_buffer = (char *)s_array._p_node;
	PifObjArrayIterator it;

	sprintf(str, "%2d:%2d ", s_array._p_free ? s_array._p_free->data[0] : -1, s_array._p_first ? s_array._p_first->data[0] : -1);
	Serial.print(str);

	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		sprintf(str, "%2d:%2d ", it->p_prev ? it->p_prev->data[0] : -1, it->p_next ? it->p_next->data[0] : -1);
		Serial.print(str);
		p_buffer += 2 * sizeof(PifObjArrayIterator) + s_array._size;
	}

	Serial.print(" U: ");
	it = s_array._p_first;
	if (it) {
		while (it) {
			sprintf(str, "%2d ", it->data[0]);
			Serial.print(str);
			it = it->p_next;
		}
	}
	else {
		Serial.print("-1 ");
	}

	Serial.print(" F: ");
	it = s_array._p_free;
	if (it) {
		while (it) {
			sprintf(str, "%2d ", it->data[0]);
			Serial.print(str);
			it = it->p_next;
		}
	}
	else {
		Serial.print("-1 ");
	}
	Serial.print("\n");
}

static TData *list_add()
{
	char *p_buffer = (char *)s_array._p_node;
	PifObjArrayIterator it, it_tmp;

	it = pifObjArray_Add(&s_array);
	if (!it) return NULL;

	for (int i = 0; i < s_array._max_count; i++) {
		it_tmp = (PifObjArrayIterator)p_buffer;
		if (it_tmp == it) {
			it->data[0] = i;
			break;
		}
		p_buffer += 2 * sizeof(PifObjArrayIterator) + s_array._size;
	}

	print();
	return (TData *)it->data;
}

static void list_delete(int16_t index)
{
	PifObjArrayIterator it;
	TData *p_data;

	it = s_array._p_first;
	while (it) {
		if (it->data[0] == index) {
			p_data = (TData *)it->data;
			break;
		}
		it = it->p_next;
	}

	pifObjArray_Remove(&s_array, p_data);

	print();
}

//The setup function is called once at startup of the sketch
void setup()
{
	char *p_buffer, str[45];
	PifObjArrayIterator it;

	Serial.begin(UART_LOG_BAUDRATE);

	Serial.println("\n\n****************************************");
	Serial.println("***           ex_obj_array           ***");
	sprintf(str, "***       %s %s       ***", __DATE__, __TIME__);
	Serial.println(str);
	Serial.println("****************************************\n");
	Serial.println(" F: U   0     1     2     3     4     5     6     7     8     9");

	if (!pifObjArray_Init(&s_array, sizeof(TData), 10, NULL)) return;
	p_buffer = (char *)s_array._p_node;
	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		it->data[0] = i;
		p_buffer += 2 * sizeof(PifObjArrayIterator) + s_array._size;
	}
	print();

	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;

	list_delete(0);
	list_delete(4);
	list_delete(2);

	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;

	list_delete(1);
	list_delete(7);
	list_delete(0);
	list_delete(3);
	list_delete(6);
	list_delete(4);
	list_delete(2);
	list_delete(5);

	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;
	if (!list_add()) return;

	list_delete(8);
	list_delete(1);
	list_delete(7);
	list_delete(0);
	list_delete(3);
	list_delete(6);
	list_delete(4);
	list_delete(2);
	list_delete(5);
	list_delete(9);
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
