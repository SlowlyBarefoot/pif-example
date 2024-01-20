/*
 * main.c
 *
 *  Created on: 2021. 11. 24.
 *      Author: wonjh
 */
#include <stdio.h>
#include <stdlib.h>

#include "storage/pif_storage_var.h"


#define STORAGE_SECTOR_SIZE		16
#define STORAGE_VOLUME			512


static PifStorageVar s_storage;
static uint8_t s_buffer[STORAGE_VOLUME];
static uint8_t s_data[256];

static BOOL actStorageRead(PifStorage* p_owner, uint8_t* dst, uint32_t src, size_t size)
{
	memcpy(dst, &s_buffer[src], size);
	return TRUE;
}

static BOOL actStorageWrite(PifStorage* p_owner, uint32_t dst, uint8_t* src, size_t size)
{
	memcpy(&s_buffer[dst], src, size);
	return TRUE;
}

void printData()
{
	int i, b, p = 0;

	printf("\n%04X: ", 0);
	for (i = 0; i < sizeof(PifStorageVarInfo); i++, p++) {
		printf("%02X ", s_buffer[p]);
	}

	for (i = 0; i < s_storage._p_info->data_info_count; i++) {
		printf("\n%04X: ", p);
		for (b = 0; b < sizeof(PifStorageVarDataInfo); b++, p++) {
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
	PifStorageDataInfoP p_data_info;
	int line;

	if (!pifStorageVar_Init(&s_storage, PIF_ID_AUTO)) { line = __LINE__; goto fail; }
	if (!pifStorageVar_AttachActStorage(&s_storage, actStorageRead, actStorageWrite)) { line = __LINE__; goto fail; }
	if (!pifStorageVar_SetMedia(&s_storage, STORAGE_SECTOR_SIZE, STORAGE_VOLUME, 5)) { line = __LINE__; goto fail; }
	if (!pifStorageVar_IsFormat(&s_storage.parent)) pifStorageVar_Format(&s_storage.parent);

	printf("info_bytes=%d\n", s_storage.__info_bytes);
	printf("info_sectors=%d\n", s_storage.__info_sectors);
	printData();

	printf("\n\nAdd first : ID=1");
	memset(s_data, 0, 256);
	memset(s_data, 1, 40);
	p_data_info = pifStorageVar_Create(&s_storage.parent, 1, 40);
	if (!p_data_info) { line = __LINE__; goto fail; }
	if (!pifStorageVar_Write(&s_storage.parent, p_data_info, s_data, 40)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nAdd last : ID=2");
	memset(s_data, 0, 256);
	memset(s_data, 2, 70);
	p_data_info = pifStorageVar_Create(&s_storage.parent, 2, 70);
	if (!p_data_info) { line = __LINE__; goto fail; }
	if (!pifStorageVar_Write(&s_storage.parent, p_data_info, s_data, 70)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nAdd last : ID=3");
	memset(s_data, 0, 256);
	memset(s_data, 3, 20);
	p_data_info = pifStorageVar_Create(&s_storage.parent, 3, 20);
	if (!p_data_info) { line = __LINE__; goto fail; }
	if (!pifStorageVar_Write(&s_storage.parent, p_data_info, s_data, 20)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nDelete first : ID=1");
	if (!pifStorageVar_Delete(&s_storage.parent, 1)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nAdd first : ID=4");
	memset(s_data, 0, 256);
	memset(s_data, 4, 30);
	p_data_info = pifStorageVar_Create(&s_storage.parent, 4, 30);
	if (!p_data_info) { line = __LINE__; goto fail; }
	if (!pifStorageVar_Write(&s_storage.parent, p_data_info, s_data, 30)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nDelete middle : ID=2");
	if (!pifStorageVar_Delete(&s_storage.parent, 2)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nAdd middle : ID=5");
	memset(s_data, 0, 256);
	memset(s_data, 5, 40);
	p_data_info = pifStorageVar_Create(&s_storage.parent, 5, 40);
	if (!p_data_info) { line = __LINE__; goto fail; }
	if (!pifStorageVar_Write(&s_storage.parent, p_data_info, s_data, 40)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nDelete middle : ID=5");
	if (!pifStorageVar_Delete(&s_storage.parent, 5)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nDelete first : ID=4");
	if (!pifStorageVar_Delete(&s_storage.parent, 4)) { line = __LINE__; goto fail; }
	printData();

	printf("\n\nDelete last : ID=3");
	if (!pifStorageVar_Delete(&s_storage.parent, 3)) { line = __LINE__; goto fail; }
	printData();
	return 0;

fail:
	printf("\nError: Line==%d\n", line);
}
