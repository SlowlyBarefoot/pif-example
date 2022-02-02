/*
 * main.c
 *
 *  Created on: 2021. 11. 24.
 *      Author: wonjh
 */
#include <stdio.h>
#include <stdlib.h>

#include "pif_storage.h"


#define STORAGE_SECTOR_SIZE		16
#define STORAGE_VOLUME			512


static PifStorage s_storage;
static uint8_t s_buffer[STORAGE_VOLUME];
static uint8_t s_data[128];


BOOL actStorageRead(uint8_t* dst, uint32_t src, size_t size)
{
	memcpy(dst, &s_buffer[src], size);
	return TRUE;
}

BOOL actStorageWrite(uint32_t dst, uint8_t* src, size_t size)
{
	memcpy(&s_buffer[dst], src, size);
	return TRUE;
}

void printData()
{
	int i, b, p = 0;

	printf("\n%04X: ", 0);
	for (i = 0; i < sizeof(PifStorageInfo); i++, p++) {
		printf("%02X ", s_buffer[p]);
	}

	for (i = 0; i < s_storage._p_info->max_data_info_count; i++) {
		printf("\n%04X: ", p);
		for (b = 0; b < sizeof(PifStorageDataInfo); b++, p++) {
			printf("%02X ", s_buffer[p]);
		}
	}

	p = s_storage.__info_sectors * s_storage._p_info->sector_size;
	for (i = s_storage.__info_sectors; i < 512 / STORAGE_SECTOR_SIZE; i++) {
		printf("\n%04X: ", i * STORAGE_SECTOR_SIZE);
		for (b = 0; b < STORAGE_SECTOR_SIZE; b++, p++) {
			printf("%02X ", s_buffer[p]);
		}
	}
}

int main()
{
	PifStorageDataInfo* p_data_info;

	pifStorage_Init(&s_storage, PIF_ID_AUTO, 5, STORAGE_SECTOR_SIZE, STORAGE_VOLUME, actStorageRead, actStorageWrite, NULL);
	if (!s_storage._is_format) pifStorage_Format(&s_storage);

	printf("info_bytes=%d\n", s_storage.__info_bytes);
	printf("info_sectors=%d\n", s_storage.__info_sectors);
	printData();

	printf("\n\nAdd first : ID=1");
	memset(s_data, 0, 256);
	memset(s_data, 1, 40);
	p_data_info = pifStorage_Alloc(&s_storage, 1, 40);
	pifStorage_Write(&s_storage, p_data_info, s_data);
	printData();

	printf("\n\nAdd last : ID=2");
	memset(s_data, 0, 256);
	memset(s_data, 2, 70);
	p_data_info = pifStorage_Alloc(&s_storage, 2, 70);
	pifStorage_Write(&s_storage, p_data_info, s_data);
	printData();

	printf("\n\nAdd last : ID=3");
	memset(s_data, 0, 256);
	memset(s_data, 3, 20);
	p_data_info = pifStorage_Alloc(&s_storage, 3, 20);
	pifStorage_Write(&s_storage, p_data_info, s_data);
	printData();

	printf("\n\nDelete first : ID=1");
	pifStorage_Free(&s_storage, 1);
	printData();

	printf("\n\nAdd first : ID=4");
	memset(s_data, 0, 256);
	memset(s_data, 4, 30);
	p_data_info = pifStorage_Alloc(&s_storage, 4, 30);
	pifStorage_Write(&s_storage, p_data_info, s_data);
	printData();

	printf("\n\nDelete middle : ID=2");
	pifStorage_Free(&s_storage, 2);
	printData();

	printf("\n\nAdd middle : ID=5");
	memset(s_data, 0, 256);
	memset(s_data, 5, 40);
	p_data_info = pifStorage_Alloc(&s_storage, 5, 40);
	pifStorage_Write(&s_storage, p_data_info, s_data);
	printData();

	printf("\n\nDelete middle : ID=5");
	pifStorage_Free(&s_storage, 5);
	printData();

	printf("\n\nDelete first : ID=4");
	pifStorage_Free(&s_storage, 4);
	printData();

	printf("\n\nDelete last : ID=3");
	pifStorage_Free(&s_storage, 3);
	printData();
	return 0;
}
