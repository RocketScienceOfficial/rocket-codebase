#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lib/maths/vector.h>
#include <lib/maths/quaternion.h>
#include <lib/geo/wgs84.h>
#include <hal/flash_driver.h>

// Ensure BOARD_FLASH_SIZE is defined
#ifndef BOARD_FLASH_SIZE
#error "BOARD_FLASH_SIZE must be defined in the board's config"
#endif

// Settings
#define PROGRAM_RESERVED_SIZE_BYTES (512UL * 1024UL) /** Space reserved for the firmware image/bootloader before the database region begins. */
#define SECTORS_COUNT_STANDING_BUFFER 16             /** Number of sectors reserved for the standing buffer. */
#define DATA_SAVE_RATE_US 20000                      /** Data save rate in microseconds. */
#define DATA_RECOVERY_MAX_FRAMES 150000              /** Maximum number of frames that can be recovered. */

// Derived constants
#define SECTORS_OFFSET_METADATA (PROGRAM_RESERVED_SIZE_BYTES / HAL_FLASH_SECTOR_SIZE)
#define SECTORS_OFFSET_STANDING_BUFFER (SECTORS_OFFSET_METADATA + 1)
#define SECTORS_OFFSET_DATA (SECTORS_OFFSET_STANDING_BUFFER + SECTORS_COUNT_STANDING_BUFFER)

// Sector-page conversions, computed once instead of repeated at every call site.
#define PAGES_PER_SECTOR (HAL_FLASH_SECTOR_SIZE / HAL_FLASH_PAGE_SIZE)
#define OFFSET_PAGES_METADATA (SECTORS_OFFSET_METADATA * PAGES_PER_SECTOR)
#define OFFSET_PAGES_STANDING_BUFFER (SECTORS_OFFSET_STANDING_BUFFER * PAGES_PER_SECTOR)
#define OFFSET_PAGES_DATA (SECTORS_OFFSET_DATA * PAGES_PER_SECTOR)

// Fills whatever flash remains on this board past the data region's start.
#define SECTORS_COUNT_DATA (((BOARD_FLASH_SIZE) / (HAL_FLASH_SECTOR_SIZE)) - (SECTORS_OFFSET_DATA))

#define STANDING_BUFFER_LENGTH (HAL_FLASH_PAGE_SIZE) // Ensure the number of elements is divisible by page size so we can easily write entire buffer
#define LANDING_BUFFER_LENGTH (HAL_FLASH_PAGE_SIZE)  // Ensure the number of elements is divisible by page size so we can easily write entire buffer

// Check if we can fit the database in the flash memory of this board.
static_assert(SECTORS_OFFSET_DATA * HAL_FLASH_SECTOR_SIZE <= BOARD_FLASH_SIZE);

struct __attribute__((__packed__)) DatabaseFrame
{
    uint16_t dt_us;
    vec3_t accRaw;
    vec3_t gyroRaw;
    vec3_t magRaw;
    geo_position_t gpsPos;
    uint8_t gpsData;
    int pressure;
    vec3_t velNED;
    vec3_t posNED;
    quat_t qNED;
    uint8_t smState;
    uint16_t batteryVoltage100;
    uint8_t ignFlags;
};

#define DATABASE_FRAME_MAGIC 0x2E

struct __attribute__((__packed__)) DatabaseFrameRaw
{
    uint8_t magic;
    DatabaseFrame frame;
    uint16_t crc;
};

struct __attribute__((__packed__)) DatabaseMetadata
{
    size_t savedFramesCount;
    size_t standingFramesCount;
};

#define DATABASE_METADATA_MAGIC 0x8F3E

struct __attribute__((__packed__)) DatabaseMetadataRaw
{
    uint16_t magic;
    DatabaseMetadata metadata;
    uint16_t crc;
};