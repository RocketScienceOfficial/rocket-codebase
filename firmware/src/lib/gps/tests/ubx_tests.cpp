#include <gtest/gtest.h>
#include <cstring>
#include "lib/gps/ubx.h"

static void feed_bytes(ubx_parser_t &parser, const uint8_t *data, size_t len, ubx_parser_status_t *lastStatus)
{
    for (size_t i = 0; i < len; i++)
    {
        *lastStatus = ubx_process_byte(&parser, data[i]);
    }
}

static void build_valid_pvt_frame(uint8_t *out, size_t *outLen)
{
    size_t i = 0;
    out[i++] = 0xb5;
    out[i++] = 0x62;
    out[i++] = 0x01; // class
    out[i++] = 0x07; // id

    const uint16_t payloadLen = sizeof(ubx_pvt_frame_t);
    out[i++] = payloadLen & 0xff;
    out[i++] = (payloadLen >> 8) & 0xff;

    for (size_t p = 0; p < payloadLen; p++)
    {
        out[i++] = (uint8_t)(p & 0xff);
    }

    uint8_t cka = 0, ckb = 0;
    for (size_t j = 2; j < i; j++)
    {
        cka += out[j];
        ckb += cka;
    }
    out[i++] = cka;
    out[i++] = ckb;

    *outLen = i;
}

TEST(UBX, process_byte_parses_valid_pvt_frame)
{
    uint8_t frame[16 + sizeof(ubx_pvt_frame_t)];
    size_t frameLen = 0;
    build_valid_pvt_frame(frame, &frameLen);

    ubx_parser_t parser{};
    ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

    feed_bytes(parser, frame, frameLen, &status);

    EXPECT_EQ(status, UBX_PARSER_STATUS_FINISHED);
    EXPECT_EQ(parser.current_frame.iTOW, 0x03020100u);
}

TEST(UBX, process_byte_rejects_frame_with_bad_checksum)
{
    uint8_t frame[16 + sizeof(ubx_pvt_frame_t)];
    size_t frameLen = 0;
    build_valid_pvt_frame(frame, &frameLen);

    frame[frameLen - 2] ^= 0xff;

    ubx_parser_t parser{};
    ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

    // Stop one byte short of the trailing ckb -- its value is checksum-derived and could
    // coincidentally equal a sync byte, which would make the state check below flaky.
    feed_bytes(parser, frame, frameLen - 1, &status);

    EXPECT_NE(status, UBX_PARSER_STATUS_FINISHED);
    EXPECT_EQ(parser.state, UBX_PARSER_STATE_SYNC1);
    EXPECT_EQ(parser.idx, 0u);
}

TEST(UBX, process_byte_resyncs_after_oversized_payload_and_parses_next_valid_frame)
{
    uint8_t oversized[6] = {0xb5, 0x62, 0x01, 0x07, 0x2c, 0x01}; // length = 300, > sizeof(parser->buffer)
    ubx_parser_t parser{};
    ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

    feed_bytes(parser, oversized, sizeof(oversized), &status);
    EXPECT_NE(status, UBX_PARSER_STATUS_FINISHED);

    uint8_t filler[300];
    memset(filler, 0x42, sizeof(filler));
    feed_bytes(parser, filler, sizeof(filler), &status);
    EXPECT_NE(status, UBX_PARSER_STATUS_FINISHED);
    EXPECT_EQ(parser.state, UBX_PARSER_STATE_SYNC1);

    uint8_t frame[16 + sizeof(ubx_pvt_frame_t)];
    size_t frameLen = 0;
    build_valid_pvt_frame(frame, &frameLen);

    feed_bytes(parser, frame, frameLen, &status);
    EXPECT_EQ(status, UBX_PARSER_STATUS_FINISHED);
}

TEST(UBX, process_byte_does_not_false_resync_on_sync_bytes_inside_payload)
{
    uint8_t frame[16 + sizeof(ubx_pvt_frame_t)];
    size_t frameLen = 0;
    build_valid_pvt_frame(frame, &frameLen);

    ASSERT_GT(sizeof(ubx_pvt_frame_t), 4u);
    frame[6] = 0xb5;
    frame[7] = 0x62;

    uint8_t cka = 0, ckb = 0;
    for (size_t j = 2; j < frameLen - 2; j++)
    {
        cka += frame[j];
        ckb += cka;
    }
    frame[frameLen - 2] = cka;
    frame[frameLen - 1] = ckb;

    ubx_parser_t parser{};
    ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

    feed_bytes(parser, frame, frameLen, &status);

    EXPECT_EQ(status, UBX_PARSER_STATUS_FINISHED);
}

TEST(UBX, process_byte_rejects_frame_with_wrong_class_or_id)
{
    uint8_t frame[16 + sizeof(ubx_pvt_frame_t)];
    size_t frameLen = 0;
    build_valid_pvt_frame(frame, &frameLen);

    frame[2] = 0x02; // wrong class
    uint8_t cka = 0, ckb = 0;
    for (size_t j = 2; j < frameLen - 2; j++)
    {
        cka += frame[j];
        ckb += cka;
    }
    frame[frameLen - 2] = cka;
    frame[frameLen - 1] = ckb;

    ubx_parser_t parser{};
    ubx_parser_status_t status = UBX_PARSER_STATUS_PARSING;

    feed_bytes(parser, frame, frameLen, &status);

    EXPECT_EQ(status, UBX_PARSER_STATUS_PARSING);
}

// ubx_create_frame's too-small-buffer check is now a SYS_CHECK, which behaves like SYS_ASSERT in
// this Debug build (asserts active) -- it aborts rather than returning.
TEST(UBX, valset_apply_aborts_when_output_buffer_is_one_byte_too_small)
{
    EXPECT_DEATH(
        {
            ubx_cfg_builder_t builder{};
            ubx_set_airborne_dynamic_model(&builder); // queues one 5-byte config entry

            uint8_t cfg[16]; // needs 17: 6 + (4 header + 5 entry) + 2
            ubx_valset_apply(&builder, cfg, sizeof(cfg));
        },
        "");
}

TEST(UBX, valset_apply_succeeds_with_exactly_sized_output_buffer)
{
    ubx_cfg_builder_t builder{};
    ubx_set_airborne_dynamic_model(&builder);

    uint8_t cfg[17];
    size_t written = ubx_valset_apply(&builder, cfg, sizeof(cfg));

    EXPECT_EQ(written, sizeof(cfg));
    EXPECT_EQ(cfg[0], 0xb5);
    EXPECT_EQ(cfg[1], 0x62);
}

TEST(UBX, valset_apply_handles_buffer_filled_close_to_capacity)
{
    // 2 * 21 entries * 5 bytes = 210 bytes, under the builder's 256-byte capacity.
    ubx_cfg_builder_t builder{};
    ubx_set_nmea_enabled_spi(&builder, true);
    ubx_set_nmea_enabled_spi(&builder, false);

    uint8_t cfg[256];
    size_t written = ubx_valset_apply(&builder, cfg, sizeof(cfg));

    EXPECT_EQ(written, 6u + 4u + 210u + 2u);
}

// ubx_valset_apply's own SYS_CHECK rejects a builder too full to fit after the 4-byte header.
TEST(UBX, valset_apply_aborts_when_queued_config_exceeds_buffer_capacity)
{
    EXPECT_DEATH(
        {
            ubx_cfg_builder_t builder{};
            for (int i = 0; i < 51; i++) // 51 * 5 = 255 bytes, over the 252 available after the 4-byte header
            {
                ubx_set_airborne_dynamic_model(&builder);
            }

            uint8_t cfg[256];
            ubx_valset_apply(&builder, cfg, sizeof(cfg));
        },
        "");
}

// ubx_valset's own SYS_CHECK now rejects an entry that would overflow the builder's 256-byte
// accumulator, instead of the old uint8_t length counter silently wrapping and corrupting it.
TEST(UBX, valset_rejects_entry_that_would_overflow_builder_capacity)
{
    EXPECT_DEATH(
        {
            ubx_cfg_builder_t builder{};
            for (int i = 0; i < 52; i++) // 52 * 5 = 260 bytes, over the builder's 256-byte capacity
            {
                ubx_set_airborne_dynamic_model(&builder);
            }
        },
        "");
}