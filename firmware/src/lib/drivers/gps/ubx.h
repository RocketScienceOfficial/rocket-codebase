#ifndef _UBX_H
#define _UBX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UBX PVT frame structure
 */
typedef struct __attribute__((packed))
{
    uint32_t iTOW;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t valid;
    uint32_t tAcc;
    int32_t nano;
    uint8_t fixType;
    uint8_t flags;
    uint8_t flags2;
    uint8_t numSV;
    int32_t lon;
    int32_t lat;
    int32_t height;
    int32_t hMSL;
    uint32_t hAcc;
    uint32_t vAcc;
    int32_t velN;
    int32_t velE;
    int32_t velD;
    int32_t gSpeed;
    int32_t headMot;
    uint32_t sAcc;
    uint32_t headAcc;
    uint16_t pDOP;
    uint16_t flags3;
    uint8_t reserved0[4];
    int32_t headVeh;
    int16_t magDec;
    uint16_t magAcc;
} ubx_pvt_frame_t;

/**
 * @brief UBX parser state
 */
typedef enum
{
    UBX_PARSER_STATE_SYNC1,
    UBX_PARSER_STATE_SYNC2,
    UBX_PARSER_STATE_HEADER,
    UBX_PARSER_STATE_PAYLOAD,
    UBX_PARSER_STATE_CKA,
    UBX_PARSER_STATE_CKB,
} ubx_parser_state_t;

/**
 * @brief UBX parser data structure
 */
typedef struct 
{
    ubx_parser_state_t state;
    uint8_t buffer[256];
    size_t idx;
    uint16_t payload_len;
    size_t payload_read;
    uint8_t ck_a;
    uint8_t ck_b;
    ubx_pvt_frame_t current_frame;
} ubx_parser_t;

/**
 * @brief UBX parser status
 */
typedef enum
{
    UBX_PARSER_STATUS_UNAVAILABLE,
    UBX_PARSER_STATUS_PARSING,
    UBX_PARSER_STATUS_FINISHED,
} ubx_parser_status_t;

/**
 * @brief Set NMEA enabled for SPI
 * 
 * @param enabled true to enable, false to disable
 */
void ubx_set_nmea_enabled_spi(bool enabled);

/**
 * @brief Set PVT enabled for SPI
 * 
 * @param enabled true to enable, false to disable
 */
void ubx_set_pvt_enabled_spi(bool enabled);

/**
 * @brief Set navigation rate
 * 
 * @param ms milliseconds
 */
void ubx_set_nav_rate(uint16_t ms);

/**
 * @brief Set airborne dynamic model
 */
void ubx_set_airborne_dynamic_model(void);

/**
 * @brief Apply configuration values
 * 
 * @param cfg buffer for configuration data
 * @param cfgLen length of configuration buffer
 * @return number of bytes written
 */
size_t ubx_valset_apply(uint8_t *cfg, uint16_t cfgLen);

/**
 * @brief Process a byte through the UBX parser
 * 
 * @param parser pointer to the UBX parser
 * @param b byte to process
 * @return parser status
 */
ubx_parser_status_t ubx_process_byte(ubx_parser_t *parser, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif