/*
 * main.c
 *
 *  Created on: 2021. 11. 1.
 *      Author: wonjh
 */

#include <stdio.h>

#include "pif_list.h"


typedef struct {
	int a;
} TestData;

static PifFixList s_list;


void print()
{
	TestData* p_data;

	PifFixListIterator it = pifFixList_Begin(&s_list);
	printf("List: ");
	if (it) {
		while (1) {
			p_data = (TestData*)it->data;
			printf("%d ", p_data->a);
			it = pifFixList_Next(it);
			if (it) {
				printf("-> ");
			}
			else {
				printf("\n");
				break;
			}
		}
	}
	else {
		printf("None\n");
	}
}

int main()
{
	TestData* p_data[6];

	if (!pifFixList_Init(&s_list, sizeof(TestData), 5)) return 0;
	print();

	for (int i = 0; i < 6; i++) {
		p_data[i] = pifFixList_AddFirst(&s_list);
		if (!p_data[i]) return 0;
		p_data[i]->a = i;
	}
	print();

	pifFixList_Remove(&s_list, p_data[4]);
	print();

	pifFixList_Remove(&s_list, p_data[0]);
	print();

	pifFixList_Remove(&s_list, p_data[2]);
	print();

	pifFixList_Remove(&s_list, p_data[3]);
	print();

	pifFixList_Remove(&s_list, p_data[1]);
	print();
	return 0;
}
