#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lib/maths/vector.h>
#include <lib/maths/quaternion.h>
#include <lib/geo/wgs84.h>
#include <board_config.h>

#define SECTORS_OFFSET_METADATA 64
#define SECTORS_OFFSET_STANDING_BUFFER 65
#define SECTORS_OFFSET_DATA 80

#define SECTORS_COUNT_STANDING_BUFFER ((SECTORS_OFFSET_DATA) - (SECTORS_OFFSET_STANDING_BUFFER))
#define SECTORS_COUNT_DATA 3500

#define DATA_SAVE_RATE_US 20000
#define DATA_RECOVERY_MAX_FRAMES 150000

#define STANDING_BUFFER_LENGTH (BOARD_FLASH_PAGE_SIZE)
#define LANDING_BUFFER_LENGTH (BOARD_FLASH_PAGE_SIZE)

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