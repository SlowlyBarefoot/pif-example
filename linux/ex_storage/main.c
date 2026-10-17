/*
 * main.c
 *
 * Two records of different sizes in an EEPROM, kept in eeprom.bin between runs: a boot counter written on every run,
 * and settings written only when they change, with room left for fields a later version may add.
 *
 *   ex_storage          count one more boot
 *   ex_storage cut N    the same, with the power cut after N page writes of the counter
 *   ex_storage delete   forget both records
 *
 * A write that is cut short never leaves half of each: the next run reads the previous count, or the new one when
 * every byte that differs had already gone in.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/pif_nvram.h"


#define MEDIA_FILE			"eeprom.bin"
#define NVRAM_CAPACITY		1024		// A 24C08
#define NVRAM_PAGE			16

#define RECORD_BOOT_COUNT	0
#define RECORD_SETTINGS		1
#define RECORD_COUNT		2


typedef struct
{
	char name[16];
	uint16_t brightness;
} Settings;


// Data size of each record, by its number. The settings get room to grow: changing a size later moves the records.
static const uint16_t c_layout[RECORD_COUNT] = {
	sizeof(uint32_t),					// RECORD_BOOT_COUNT
	48									// RECORD_SETTINGS
};

static PifNvram s_nvram;
static uint8_t s_media[NVRAM_CAPACITY];
static int s_cut_after = -1;			// Page writes that go through before the power is cut, -1 for never


static uint32_t _timer1us(void)
{
	return (uint32_t)clock() * (1000000 / CLOCKS_PER_SEC);
}

static BOOL _actRead(PifNvram* p_owner, uint8_t* p_dst, uint32_t src, size_t size)
{
	(void)p_owner;

	memcpy(p_dst, s_media + src, size);
	return TRUE;
}

static BOOL _actWrite(PifNvram* p_owner, uint32_t dst, uint8_t* p_src, size_t size)
{
	(void)p_owner;

	if (s_cut_after == 0) {
		// Half a page goes in as the power goes, and nothing after it.
		memcpy(s_media + dst, p_src, size / 2);
		printf("  power cut at 0x%03X\n", (unsigned)dst);
		return FALSE;
	}
	if (s_cut_after > 0) s_cut_after--;
	memcpy(s_media + dst, p_src, size);
	return TRUE;
}

static void _loadMedia(void)
{
	FILE* p_file = fopen(MEDIA_FILE, "rb");

	// A new EEPROM reads as all ones.
	memset(s_media, 0xFF, sizeof(s_media));
	if (!p_file) return;
	if (fread(s_media, 1, sizeof(s_media), p_file) != sizeof(s_media)) memset(s_media, 0xFF, sizeof(s_media));
	fclose(p_file);
}

static void _saveMedia(void)
{
	FILE* p_file = fopen(MEDIA_FILE, "wb");

	if (!p_file) return;
	fwrite(s_media, 1, sizeof(s_media), p_file);
	fclose(p_file);
}

int main(int argc, char* argv[])
{
	Settings settings;
	uint32_t boot_count;
	uint16_t size;

	if (argc > 2 && !strcmp(argv[1], "cut")) s_cut_after = atoi(argv[2]);

	if (!pif_Init(_timer1us)) return 1;
	_loadMedia();

	if (!pifNvram_Init(&s_nvram, PIF_ID_AUTO)) goto fail;
	if (!pifNvram_AttachActMedia(&s_nvram, _actRead, _actWrite)) goto fail;
	if (!pifNvram_SetLayout(&s_nvram, NVRAM_CAPACITY, NVRAM_PAGE, c_layout, RECORD_COUNT, NM_RECORD)) goto fail;

	if (argc > 1 && !strcmp(argv[1], "delete")) {
		if (!pifNvram_DeleteRecord(&s_nvram, RECORD_BOOT_COUNT)) goto fail;
		if (!pifNvram_DeleteRecord(&s_nvram, RECORD_SETTINGS)) goto fail;
		printf("Records deleted\n");
		goto done;
	}

	// Defaults first: a record that is missing leaves them, and one written by an older, shorter Settings keeps
	// the fields it does not have.
	memset(&settings, 0, sizeof(settings));
	strcpy(settings.name, "pif");
	settings.brightness = 50;
	if (pifNvram_ReadRecord(&s_nvram, RECORD_SETTINGS, (uint8_t*)&settings, sizeof(settings), &size)) {
		printf("Settings read: %u bytes, %s at %u%%\n", size, settings.name, settings.brightness);
	}
	else if (pif_error == E_CANNOT_FOUND) {
		printf("No settings yet, defaults saved\n");
		if (!pifNvram_WriteRecord(&s_nvram, RECORD_SETTINGS, (uint8_t*)&settings, sizeof(settings))) goto fail;
	}
	else goto fail;

	boot_count = 0;
	if (!pifNvram_ReadRecord(&s_nvram, RECORD_BOOT_COUNT, (uint8_t*)&boot_count, sizeof(boot_count), NULL)
			&& pif_error != E_CANNOT_FOUND) goto fail;
	boot_count++;
	printf("Boot count: %lu\n", (unsigned long)boot_count);

	// No timer is attached, so the write is over when it returns.
	if (!pifNvram_WriteRecord(&s_nvram, RECORD_BOOT_COUNT, (uint8_t*)&boot_count, sizeof(boot_count))) {
		printf("Write cut short: the next run reads the last count that went in whole\n");
	}

done:
	_saveMedia();
	pifNvram_Clear(&s_nvram);
	return 0;

fail:
	printf("Error: %d\n", pif_error);
	pifNvram_Clear(&s_nvram);
	return 1;
}
