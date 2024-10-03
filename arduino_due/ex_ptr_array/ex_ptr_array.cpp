// Do not remove the include below
#include "ex_ptr_array.h"

#include "core/pif_ptr_array.h"


#define UART_LOG_BAUDRATE		115200

typedef struct {
	uint8_t data[8];
} TData;

static PifPtrArray s_array;
static TData data[10];


void print()
{
	char str[10];
	char *p_buffer = (char *)s_array._p_node;
	PifPtrArrayIterator it;

	sprintf(str, "%2d:%2d ", s_array._p_free ? ((TData*)s_array._p_free->p_data)->data[0] : -1, s_array._p_first ? ((TData*)s_array._p_first->p_data)->data[0] : -1);
	Serial.print(str);

	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifPtrArrayIterator)p_buffer;
		sprintf(str, "%2d:%2d ", it->p_prev ? ((TData*)it->p_prev->p_data)->data[0] : -1, it->p_next ? ((TData*)it->p_next->p_data)->data[0] : -1);
		Serial.print(str);
		p_buffer += sizeof(PifPtrArrayNode);
	}

	Serial.print(" U: ");
	it = s_array._p_first;
	if (it) {
		while (it) {
			sprintf(str, "%2d ", ((TData*)it->p_data)->data[0]);
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
			sprintf(str, "%2d ", ((TData*)it->p_data)->data[0]);
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
	PifPtrArrayIterator it, it_tmp;

	it = pifPtrArray_Add(&s_array);
	if (!it) return NULL;

	print();
	return (TData *)it->p_data;
}

static void list_delete(int16_t index)
{
	PifPtrArrayIterator it;
	TData *p_data;

	it = s_array._p_first;
	while (it) {
		if (((TData*)it->p_data)->data[0] == index) {
			p_data = (TData *)it->p_data;
			break;
		}
		it = it->p_next;
	}

	pifPtrArray_Remove(&s_array, p_data);

	print();
}

//The setup function is called once at startup of the sketch
void setup()
{
	char *p_buffer, str[45];
	PifPtrArrayIterator it;

	Serial.begin(UART_LOG_BAUDRATE);

	Serial.println("\n\n****************************************");
	Serial.println("***           ex_ptr_array           ***");
	sprintf(str, "***       %s %s       ***", __DATE__, __TIME__);
	Serial.println(str);
	Serial.println("****************************************\n");
	Serial.println(" F: U   0     1     2     3     4     5     6     7     8     9");

	if (!pifPtrArray_Init(&s_array, 10, NULL)) return;
	p_buffer = (char *)s_array._p_node;
	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifPtrArrayIterator)p_buffer;
		it->p_data = (char *)(data + i);
		data[i].data[0] = i;
		p_buffer += sizeof(PifPtrArrayNode);
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
