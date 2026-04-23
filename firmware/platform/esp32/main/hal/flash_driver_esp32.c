#include "hal/flash_driver.h"
#include "esp_flash.h"

void hal_flash_init(void)
{
    esp_flash_init(esp_flash_default_chip);
}

void hal_flash_read(size_t offset, uint8_t *dst, size_t size)
{
    esp_flash_read(esp_flash_default_chip, dst, offset, size);
}

void hal_flash_write_pages(size_t offsetPages, const uint8_t *buffer, size_t pagesCount)
{
    size_t offset_bytes = offsetPages * BOARD_FLASH_PAGE_SIZE;
    size_t size_bytes = pagesCount * BOARD_FLASH_PAGE_SIZE;

    esp_flash_write(esp_flash_default_chip, buffer, offset_bytes, size_bytes);
}

void hal_flash_erase_sectors(size_t sectorsOffset, size_t sectorsCount)
{
    size_t offset_bytes = sectorsOffset * BOARD_FLASH_SECTOR_SIZE;
    size_t size_bytes = sectorsCount * BOARD_FLASH_SECTOR_SIZE;

    esp_flash_erase_region(esp_flash_default_chip, offset_bytes, size_bytes);
}