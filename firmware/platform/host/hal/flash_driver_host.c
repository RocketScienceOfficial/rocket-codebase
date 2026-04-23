#include "hal/flash_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLASH_FILEPATH "flash.bin"

static uint8_t g_flashMemory[BOARD_FLASH_SIZE];
static FILE *g_flashFile = NULL;

static void _init_file(void)
{
    g_flashFile = fopen(FLASH_FILEPATH, "r+b");

    if (g_flashFile == NULL)
    {
        g_flashFile = fopen(FLASH_FILEPATH, "w+b");

        if (g_flashFile != NULL)
        {
            memset(g_flashMemory, 0xFF, BOARD_FLASH_SIZE);
            fwrite(g_flashMemory, 1, BOARD_FLASH_SIZE, g_flashFile);
            fflush(g_flashFile);
        }
    }
    else
    {
        fread(g_flashMemory, 1, BOARD_FLASH_SIZE, g_flashFile);
    }
}

static void _sync_to_file(size_t offset, size_t length)
{
    if (g_flashFile)
    {
        fseek(g_flashFile, (long)offset, SEEK_SET);
        fwrite(&g_flashMemory[offset], 1, length, g_flashFile);
        fflush(g_flashFile);
    }
}

void hal_flash_init(void)
{
    _init_file();
}

void hal_flash_read(size_t offset, uint8_t *dst, size_t size)
{
    memcpy(dst, &g_flashMemory[offset], size);
}

void hal_flash_write_pages(size_t offsetPages, const uint8_t *buffer, size_t pagesCount)
{
    size_t byte_offset = offsetPages * BOARD_FLASH_PAGE_SIZE;
    size_t byte_length = pagesCount * BOARD_FLASH_PAGE_SIZE;

    for (size_t i = 0; i < byte_length; i++)
    {
        g_flashMemory[byte_offset + i] &= buffer[i];
    }

    _sync_to_file(byte_offset, byte_length);
}

void hal_flash_erase_sectors(size_t sectorsOffset, size_t sectorsCount)
{
    size_t byte_offset = sectorsOffset * BOARD_FLASH_SECTOR_SIZE;
    size_t byte_length = sectorsCount * BOARD_FLASH_SECTOR_SIZE;

    memset(&g_flashMemory[byte_offset], 0xFF, byte_length);

    _sync_to_file(byte_offset, byte_length);
}