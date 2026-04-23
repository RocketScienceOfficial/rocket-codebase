#include "hal/flash_driver.h"
#include "hardware/flash.h"
#include "hardware/timer.h"
#include "pico/flash.h"
#include <string.h>

typedef struct
{
    uint32_t offset;
    const uint8_t *buffer;
    size_t size;
} flash_args_t;

void hal_flash_init(void)
{
}

void hal_flash_read(size_t offset, uint8_t *dst, size_t size)
{
    memcpy(dst, (const uint8_t *)(XIP_BASE + offset), size);
}

static void __not_in_flash_func(_flash_write_helper)(void *arg)
{
    flash_args_t *args = (flash_args_t *)arg;
    flash_range_program(args->offset, args->buffer, args->size);
}

void hal_flash_write_pages(size_t offsetPages, const uint8_t *buffer, size_t pagesCount)
{
    uint32_t offset = offsetPages * FLASH_PAGE_SIZE;
    size_t size = pagesCount * FLASH_PAGE_SIZE;

    flash_args_t args = {offset, buffer, size};
    flash_safe_execute(_flash_write_helper, &args, 1000);

    busy_wait_us(10);
}

static void __not_in_flash_func(_flash_erase_helper)(void *arg)
{
    flash_args_t *args = (flash_args_t *)arg;
    flash_range_erase(args->offset, args->size);
}

void hal_flash_erase_sectors(size_t sectorsOffset, size_t sectorsCount)
{
    uint32_t offset = sectorsOffset * FLASH_SECTOR_SIZE;
    size_t size = sectorsCount * FLASH_SECTOR_SIZE;

    flash_args_t args = {offset, NULL, size};
    flash_safe_execute(_flash_erase_helper, &args, 1000);

    busy_wait_us(10);
}