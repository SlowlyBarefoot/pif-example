// Do not remove the include below
#include "ex_ptr_array.h"

#include "core/pif_ptr_array.h"


#define UART_LOG_BAUDRATE		115200


static PifPtrArray s_array;


void list_print(char *mode, int index)
{
	char str[20];
	char *p_buffer = (char *)s_array._p_node;
	PifPtrArrayIterator it;

	sprintf(str, "%s:%2d  %2d:%2d ", mode, index, s_array._p_free ? s_array._p_free - s_array._p_node : -1, s_array._p_first ? s_array._p_first - s_array._p_node : -1);
	Serial.print(str);

	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifPtrArrayIterator)p_buffer;
		sprintf(str, "%2d:%2d ", it->p_prev ? it->p_prev - s_array._p_node : -1, it->p_next ? it->p_next - s_array._p_node : -1);
		Serial.print(str);
		p_buffer += sizeof(PifPtrArrayNode);
	}

	sprintf(str, "  C:%2d  U: ", s_array._count);
	Serial.print(str);
	it = s_array._p_first;
	if (it) {
		while (it) {
			sprintf(str, "%2d ", it - s_array._p_node);
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
			sprintf(str, "%2d ", it - s_array._p_node);
			Serial.print(str);
			it = it->p_next;
		}
	}
	else {
		Serial.print("-1 ");
	}
	Serial.print("\n");
}

static bool list_add()
{
	char *p_buffer = (char *)s_array._p_node;
	PifPtrArrayIterator it, it_tmp;

	it = pifPtrArray_Add(&s_array, NULL);
	if (!it) return false;

	list_print("Add ", it - s_array._p_node);
	return true;
}

static void list_delete(int16_t index)
{
	PifPtrArrayIterator it;

	it = s_array._p_first;
	while (it) {
		if (it - s_array._p_node == index) break;
		it = it->p_next;
	}

	pifPtrArray_Remove(&s_array, it);

	list_print("Del ", index);
}

int list_find(int index)
{
	int i;
	PifPtrArrayIterator it = s_array._p_first;
	for (i = 0; i < index; i++) {
		it = it->p_next;
	}
	return it - s_array._p_node;
}

//The setup function is called once at startup of the sketch
void setup()
{
	char *p_buffer, str[45];
	int i, k, mode = 0, num;
	PifPtrArrayIterator it;

	Serial.begin(UART_LOG_BAUDRATE);

	Serial.println("\n\n****************************************");
	Serial.println("***           ex_ptr_array           ***");
	sprintf(str, "***       %s %s       ***", __DATE__, __TIME__);
	Serial.println(str);
	Serial.println("****************************************\n");
	Serial.println(" F: U   0     1     2     3     4     5     6     7     8     9");

	if (!pifPtrArray_Init(&s_array, 10, NULL)) return;
	list_print("Init", 0);

	for (i = 0; i < 1000; i++) {
		if (mode) {
			num = rand() % s_array._count;
			for (k = 0; k < num; k++) {
				list_delete(list_find(rand() % s_array._count));
			}
		}
		else {
			num = rand() % 6 + 1;
			if (num > 10 - s_array._count) num = 10 - s_array._count;
			for (k = 0; k < num; k++) {
				list_add();
			}
		}
		mode ^= 1;
	}
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
