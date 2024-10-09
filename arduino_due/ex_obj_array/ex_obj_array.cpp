// Do not remove the include below
#include "ex_obj_array.h"

#include "core/pif_obj_array.h"


#define UART_LOG_BAUDRATE		115200


static PifObjArray s_array;


void list_print(char *mode, int index)
{
	char str[20];
	char *p_buffer = (char *)s_array._p_node;
	PifObjArrayIterator it;

	sprintf(str, "%s:%2d  %2d:%2d ", mode, index, s_array._p_free ? s_array._p_free->data[0] : -1, s_array._p_first ? s_array._p_first->data[0] : -1);
	Serial.print(str);

	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		sprintf(str, "%2d:%2d ", it->p_prev ? it->p_prev->data[0] : -1, it->p_next ? it->p_next->data[0] : -1);
		Serial.print(str);
		p_buffer += 2 * sizeof(PifObjArrayIterator) + s_array._size;
	}

	sprintf(str, "  C:%2d  U: ", s_array._count);
	Serial.print(str);
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

static char *list_add()
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

	list_print("Add ", it_tmp->data[0]);
	return (char *)it->data;
}

static void list_delete(int16_t index)
{
	PifObjArrayIterator it;
	char *p_data;

	it = s_array._p_first;
	while (it) {
		if (it->data[0] == index) {
			p_data = it->data;
			break;
		}
		it = it->p_next;
	}

	pifObjArray_Remove(&s_array, p_data);

	list_print("Del ", index);
}

int list_find(int index)
{
	int i;
	PifObjArrayIterator it = s_array._p_first;
	for (i = 0; i < index; i++) {
		it = it->p_next;
	}
	return it->data[0];
}

//The setup function is called once at startup of the sketch
void setup()
{
	char *p_buffer, str[45];
	int i, k, mode = 0, num;
	PifObjArrayIterator it;

	Serial.begin(UART_LOG_BAUDRATE);

	Serial.println("\n\n****************************************");
	Serial.println("***           ex_obj_array           ***");
	sprintf(str, "***       %s %s       ***", __DATE__, __TIME__);
	Serial.println(str);
	Serial.println("****************************************\n");
	Serial.println("         F: U   0     1     2     3     4     5     6     7     8     9");

	if (!pifObjArray_Init(&s_array, sizeof(char), 10, NULL)) return;
	p_buffer = (char *)s_array._p_node;
	for (int i = 0; i < s_array._max_count; i++) {
		it = (PifObjArrayIterator)p_buffer;
		it->data[0] = i;
		p_buffer += 2 * sizeof(PifObjArrayIterator) + s_array._size;
	}
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
