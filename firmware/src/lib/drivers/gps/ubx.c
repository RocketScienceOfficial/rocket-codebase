#include "ubx.h"
#include <lib/debug/sys_assert.h>
#include <string.h>

#define UBX_PREAMBLE_SYNC_CHAR_1 0xb5
#define UBX_PREAMBLE_SYNC_CHAR_2 0x62

static uint8_t g_cfgValBuffer[256];
static uint8_t g_cfgValBufferLen;

static void ubx_valset(uint32_t id, uint32_t value, uint8_t size)
{
    g_cfgValBuffer[g_cfgValBufferLen++] = (id >> 0) & 0xff;
    g_cfgValBuffer[g_cfgValBufferLen++] = (id >> 8) & 0xff;
    g_cfgValBuffer[g_cfgValBufferLen++] = (id >> 16) & 0xff;
    g_cfgValBuffer[g_cfgValBufferLen++] = (id >> 24) & 0xff;

    for (size_t j = 0; j < size; j++)
    {
        g_cfgValBuffer[g_cfgValBufferLen++] = (value >> (j * 8)) & 0xff;
    }
}

static void ubx_checksum_update(uint8_t *cka, uint8_t *ckb, uint8_t b)
{
    *cka += b;
    *ckb += *cka;
}

static void ubx_checksum_calculate(uint8_t *cka, uint8_t *ckb, const uint8_t *buffer, uint8_t len)
{
    *cka = 0;
    *ckb = 0;

    for (uint8_t i = 0; i < len; i++)
    {
        ubx_checksum_update(cka, ckb, buffer[i]);
    }

    *cka &= 0xff;
    *ckb &= 0xff;
}

void ubx_set_nmea_enabled_spi(bool enabled)
{
    ubx_valset(0x10790001, 1, 1);

    ubx_valset(0x107a0001, enabled ? 0 : 1, 1);
    ubx_valset(0x107a0002, enabled ? 1 : 0, 1);

    ubx_valset(0x209100aa, enabled ? 1 : 0, 1);
    ubx_valset(0x209100e1, enabled ? 1 : 0, 1);
    ubx_valset(0x209100be, enabled ? 1 : 0, 1);
    ubx_valset(0x209100cd, enabled ? 1 : 0, 1);
    ubx_valset(0x209100b9, enabled ? 1 : 0, 1);
    ubx_valset(0x209100d2, enabled ? 1 : 0, 1);
    ubx_valset(0x209100c3, enabled ? 1 : 0, 1);
    ubx_valset(0x209100d7, enabled ? 1 : 0, 1);
    ubx_valset(0x209100c8, enabled ? 1 : 0, 1);
    ubx_valset(0x20910404, enabled ? 1 : 0, 1);
    ubx_valset(0x209100af, enabled ? 1 : 0, 1);
    ubx_valset(0x209100eb, enabled ? 1 : 0, 1);
    ubx_valset(0x209100b4, enabled ? 1 : 0, 1);
    ubx_valset(0x209100dc, enabled ? 1 : 0, 1);
    ubx_valset(0x209100f0, enabled ? 1 : 0, 1);
    ubx_valset(0x209100f5, enabled ? 1 : 0, 1);
    ubx_valset(0x209100fa, enabled ? 1 : 0, 1);
    ubx_valset(0x2091025d, enabled ? 1 : 0, 1);
}

void ubx_set_pvt_enabled_spi(bool enabled)
{
    ubx_valset(0x2091000a, enabled ? 1 : 0, 1);
}

void ubx_set_nav_rate(uint16_t ms)
{
    ubx_valset(0x30210001, ms, 2);
    ubx_valset(0x30210002, 1, 2);
}

void ubx_set_airborne_dynamic_model(void)
{
    ubx_valset(0x20110021, 8, 1);
}

static size_t ubx_create_frame(uint8_t *frameBuffer, uint16_t frameBufferLen, uint8_t class, uint8_t id, const uint8_t *payload, uint16_t length)
{
    if (frameBufferLen < 6 + length + 2)
    {
        SYS_ASSERT(false);

        return 0;
    }

    frameBuffer[0] = UBX_PREAMBLE_SYNC_CHAR_1;
    frameBuffer[1] = UBX_PREAMBLE_SYNC_CHAR_2;
    frameBuffer[2] = class;
    frameBuffer[3] = id;
    frameBuffer[4] = length & 0xff;
    frameBuffer[5] = (length >> 8) & 0xff;

    memcpy(frameBuffer + 6, payload, length);

    uint8_t cka, ckb;
    ubx_checksum_calculate(&cka, &ckb, frameBuffer + 2, 4 + length);

    frameBuffer[6 + length + 0] = cka;
    frameBuffer[6 + length + 1] = ckb;

    return 6 + length + 2;
}

size_t ubx_valset_apply(uint8_t *cfg, uint16_t cfgLen)
{
    uint8_t buffer[256];
    size_t i = 0;

    buffer[i++] = 0x00;
    buffer[i++] = 0x07;
    buffer[i++] = 0x00;
    buffer[i++] = 0x00;

    SYS_ASSERT(sizeof(buffer) - i >= g_cfgValBufferLen);

    memcpy(buffer + i, g_cfgValBuffer, g_cfgValBufferLen);
    i += g_cfgValBufferLen;
    g_cfgValBufferLen = 0;

    return ubx_create_frame(cfg, cfgLen, 0x06, 0x8a, buffer, i);
}

static bool ubx_handle_frame(ubx_pvt_frame_t *frame, const uint8_t *buffer, size_t bufLen)
{
    uint8_t class = buffer[0];
    uint8_t id = buffer[1];
    const uint8_t *payload = &buffer[4];
    size_t payload_len = bufLen - 4;

    if (class == 0x01 && id == 0x07 && payload_len == sizeof(ubx_pvt_frame_t))
    {
        memcpy(frame, payload, sizeof(ubx_pvt_frame_t));

        return true;
    }

    return false;
}

static void ubx_reset_parser(ubx_parser_t *parser)
{
    parser->state = UBX_PARSER_STATE_SYNC1;
    parser->idx = 0;
    parser->payload_len = 0;
    parser->payload_read = 0;
    parser->ck_a = 0;
    parser->ck_b = 0;
}

ubx_parser_status_t ubx_process_byte(ubx_parser_t *parser, uint8_t b)
{
    switch (parser->state)
    {
    case UBX_PARSER_STATE_SYNC1:
    {
        if (b == UBX_PREAMBLE_SYNC_CHAR_1)
        {
            parser->state = UBX_PARSER_STATE_SYNC2;
        }
        else
        {
            return UBX_PARSER_STATUS_UNAVAILABLE;
        }
        break;
    }
    case UBX_PARSER_STATE_SYNC2:
    {
        if (b == UBX_PREAMBLE_SYNC_CHAR_2)
        {
            parser->state = UBX_PARSER_STATE_HEADER;
            parser->idx = 0;
            parser->ck_a = 0;
            parser->ck_b = 0;
        }
        else
        {
            ubx_reset_parser(parser);
        }
        break;
    }
    case UBX_PARSER_STATE_HEADER:
    {
        if (parser->idx >= sizeof(parser->buffer))
        {
            ubx_reset_parser(parser);
            break;
        }

        parser->buffer[parser->idx++] = b;

        ubx_checksum_update(&parser->ck_a, &parser->ck_b, b);

        if (parser->idx == 4)
        {
            parser->state = UBX_PARSER_STATE_PAYLOAD;
            parser->payload_len = parser->buffer[2] | (parser->buffer[3] << 8);
            parser->payload_read = 0;
        }
        break;
    }
    case UBX_PARSER_STATE_PAYLOAD:
    {
        if (parser->idx >= sizeof(parser->buffer))
        {
            ubx_reset_parser(parser);
            break;
        }

        parser->buffer[parser->idx++] = b;
        parser->payload_read++;

        ubx_checksum_update(&parser->ck_a, &parser->ck_b, b);

        if (parser->payload_read == parser->payload_len)
        {
            parser->state = UBX_PARSER_STATE_CKA;
        }
        break;
    }
    case UBX_PARSER_STATE_CKA:
    {
        if (b == parser->ck_a)
        {
            parser->state = UBX_PARSER_STATE_CKB;
        }
        else
        {
            ubx_reset_parser(parser);
        }
        break;
    }
    case UBX_PARSER_STATE_CKB:
    {
        ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

        if (b == parser->ck_b)
        {
            if (ubx_handle_frame(&parser->current_frame, parser->buffer, parser->idx))
            {
                status = UBX_PARSER_STATUS_FINISHED;
            }
        }

        ubx_reset_parser(parser);

        return status;
    }
    }

    return UBX_PARSER_STATUS_PARSING;
}