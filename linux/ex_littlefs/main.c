/*
 * main.c
 *
 * littlefs on an SPI NOR flash through pif_nor_flash and pif_littlefs. There is no SPI here, so the flash is a
 * W25Q80 (1MB) played by the SPI port itself, kept in nor_flash.bin between runs. On a board the port actions drive
 * the real SPI and the rest stays as it is.
 *
 *   ex_littlefs           count one more boot and write a log line
 *   ex_littlefs cut N     the same, with the power cut after N page programs
 *   ex_littlefs format    start over with an empty file system
 *
 * littlefs is not part of pif: clone https://github.com/littlefs-project/littlefs next to pif.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/pif_littlefs.h"


#define MEDIA_FILE			"nor_flash.bin"
#define FLASH_CAPACITY		0x100000UL
#define FLASH_PAGE			256
#define FLASH_SECTOR		4096


static PifSpiPort s_spi_port;
static PifNorFlash s_flash;
static PifLittlefs s_bd;
static lfs_t s_lfs;

// The device the port plays.
static uint8_t s_media[FLASH_CAPACITY];
static const uint8_t c_jedec_id[3] = { 0xEF, 0x40, 0x14 };
static BOOL s_wel;
static int s_cut_after = -1;			// Programs that go through before the power is cut, -1 for never


static uint32_t _timer1us(void)
{
	return (uint32_t)clock() * (1000000 / CLOCKS_PER_SEC);
}

// Commands without data: write enable and erases. Every erase and program is over at once here, so the device is
// never busy.
static void _actTransfer(PifSpiDevice *p_owner, uint8_t* p_write, uint8_t* p_read, size_t size)
{
	uint32_t addr;

	(void)p_owner;
	(void)p_read;

	switch (p_write[0]) {
	case 0x06:
		s_wel = TRUE;
		break;

	case 0x20:
		if (!s_wel || size != 4 || s_cut_after == 0) break;
		addr = ((uint32_t)p_write[1] << 16 | p_write[2] << 8 | p_write[3]) % FLASH_CAPACITY;
		memset(s_media + addr / FLASH_SECTOR * FLASH_SECTOR, 0xFF, FLASH_SECTOR);
		s_wel = FALSE;
		break;
	}
}

// Commands that read: JEDEC ID, status and data.
static BOOL _actRead(PifSpiDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, size_t size)
{
	uint32_t addr;

	(void)p_owner;

	if (isize == 1 && iaddr == 0x9F) {
		memcpy(p_data, c_jedec_id, size < 3 ? size : 3);
	}
	else if (isize == 1 && iaddr == 0x05) {
		p_data[0] = s_wel ? 0x02 : 0x00;
	}
	else if (isize == 4 && (iaddr >> 24) == 0x03) {
		addr = iaddr & 0xFFFFFF;
		if (addr + size > FLASH_CAPACITY) return FALSE;
		memcpy(p_data, s_media + addr, size);
	}
	else return FALSE;
	return TRUE;
}

// Page program, which only clears bits.
static BOOL _actWrite(PifSpiDevice *p_owner, uint32_t iaddr, uint8_t isize, uint8_t* p_data, size_t size)
{
	uint32_t addr;
	size_t i;

	(void)p_owner;

	if (isize != 4 || (iaddr >> 24) != 0x02) return FALSE;
	if (!s_wel) return TRUE;
	s_wel = FALSE;
	if (s_cut_after == 0) return TRUE;
	if (s_cut_after > 0 && --s_cut_after == 0) printf("  power cut\n");

	addr = iaddr & 0xFFFFFF;
	for (i = 0; i < size; i++) {
		s_media[addr / FLASH_PAGE * FLASH_PAGE + (addr + i) % FLASH_PAGE] &= p_data[i];
	}
	return TRUE;
}

static void _loadMedia(void)
{
	FILE* p_file = fopen(MEDIA_FILE, "rb");

	// A new flash reads as all ones.
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

static int _countBoot(void)
{
	lfs_file_t file;
	uint32_t boot_count = 0;
	char line[48];
	int err, len;

	err = lfs_file_open(&s_lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
	if (err) return err;
	lfs_file_read(&s_lfs, &file, &boot_count, sizeof(boot_count));
	boot_count++;
	lfs_file_rewind(&s_lfs, &file);
	lfs_file_write(&s_lfs, &file, &boot_count, sizeof(boot_count));
	// The file is written only here: until then the old count stays as it is.
	err = lfs_file_close(&s_lfs, &file);
	if (err) return err;
	printf("Boot count: %lu\n", (unsigned long)boot_count);

	err = lfs_file_open(&s_lfs, &file, "log.txt", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
	if (err) return err;
	len = snprintf(line, sizeof(line), "boot %lu\n", (unsigned long)boot_count);
	lfs_file_write(&s_lfs, &file, line, len);
	return lfs_file_close(&s_lfs, &file);
}

static void _listFiles(void)
{
	lfs_dir_t dir;
	struct lfs_info info;

	if (lfs_dir_open(&s_lfs, &dir, "/")) return;
	printf("Files:\n");
	while (lfs_dir_read(&s_lfs, &dir, &info) > 0) {
		if (info.type == LFS_TYPE_REG) printf("  %-12s %lu bytes\n", info.name, (unsigned long)info.size);
	}
	lfs_dir_close(&s_lfs, &dir);
	printf("Blocks in use: %ld of %lu\n", (long)lfs_fs_size(&s_lfs), (unsigned long)s_bd.cfg.block_count);
}

int main(int argc, char* argv[])
{
	int err;

	if (argc > 2 && !strcmp(argv[1], "cut")) s_cut_after = atoi(argv[2]);

	if (!pif_Init(_timer1us)) return 1;
	_loadMedia();
	if (argc > 1 && !strcmp(argv[1], "format")) memset(s_media, 0xFF, sizeof(s_media));

	if (!pifSpiPort_Init(&s_spi_port, PIF_ID_AUTO, 1)) goto fail;
	s_spi_port.act_transfer = _actTransfer;
	s_spi_port.act_read = _actRead;
	s_spi_port.act_write = _actWrite;

	if (!pifNorFlash_Init(&s_flash, PIF_ID_AUTO, &s_spi_port, NULL, 0)) goto fail;
	printf("Flash %02X %02X %02X, %lu bytes\n", s_flash._jedec_id[0], s_flash._jedec_id[1], s_flash._jedec_id[2],
			(unsigned long)s_flash._capacity);
	if (!pifLittlefs_Init(&s_bd, &s_flash, 0, 0)) goto fail;

	// The first mount of a new flash fails, and that is when it is formatted.
	err = lfs_mount(&s_lfs, &s_bd.cfg);
	if (err) {
		printf("Formatting\n");
		err = lfs_format(&s_lfs, &s_bd.cfg);
		if (!err) err = lfs_mount(&s_lfs, &s_bd.cfg);
	}
	if (err) {
		printf("Mount error: %d\n", err);
		goto fail;
	}

	err = _countBoot();
	if (err) printf("Write error: %d\n", err);
	if (s_cut_after != 0) _listFiles();
	lfs_unmount(&s_lfs);

	_saveMedia();
	pifNorFlash_Clear(&s_flash);
	pifSpiPort_Clear(&s_spi_port);
	return 0;

fail:
	printf("Error: %d\n", pif_error);
	pifNorFlash_Clear(&s_flash);
	pifSpiPort_Clear(&s_spi_port);
	return 1;
}
