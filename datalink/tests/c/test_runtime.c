#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "datalink.h"

static int g_failed = 0;
static int g_passed = 0;

#define ASSERT_TRUE(cond, msg)                        \
    do                                                \
    {                                                 \
        if (!(cond))                                  \
        {                                             \
            printf("[FAIL] %s: %s\n", __func__, msg); \
            g_failed++;                               \
            return;                                   \
        }                                             \
    } while (0)

static void run_test(const char *name, void (*fn)(void))
{
    int before = g_failed;
    fn();

    if (g_failed == before)
    {
        g_passed++;
        printf("[PASS] %s\n", name);
    }
    else
    {
        printf("[FAIL] %s\n", name);
    }
}

#define RUN_TEST(func) run_test(#func, func)

static void fill_payload(datalink_message_t *msg, uint8_t seed)
{
    for (uint8_t i = 0; i < msg->len; i++)
    {
        msg->payload[i] = (uint8_t)(seed + i);
    }
}

static void assert_canary_untouched(const uint8_t *buffer, int start, int end, uint8_t canary, const char *msg)
{
    for (int i = start; i < end; i++)
    {
        ASSERT_TRUE(buffer[i] == canary, msg);
    }
}

static void test_crc16_known_vector(void)
{
    const uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc = datalink_crc16_mcrf4xx_calculate(data, (int)sizeof(data));
    ASSERT_TRUE(crc == 0xC66E, "CRC16 known-vector mismatch");
}

static void test_cobs_known_vector_encode_decode(void)
{
    const uint8_t data[] = {0x11, 0x22, 0x00, 0x33};
    const uint8_t expected_encoded[] = {0x03, 0x11, 0x22, 0x02, 0x33};
    uint8_t encoded[16] = {0};
    uint8_t decoded[16] = {0};

    int encoded_len = datalink_cobs_encode(data, (int)sizeof(data), encoded, (int)sizeof(encoded));
    ASSERT_TRUE(encoded_len == (int)sizeof(expected_encoded), "unexpected COBS encoded length");
    ASSERT_TRUE(memcmp(encoded, expected_encoded, sizeof(expected_encoded)) == 0, "COBS known-vector encode mismatch");

    int decoded_len = datalink_cobs_decode(encoded, encoded_len, decoded, (int)sizeof(decoded));
    ASSERT_TRUE(decoded_len == (int)sizeof(data), "unexpected COBS decoded length");
    ASSERT_TRUE(memcmp(decoded, data, sizeof(data)) == 0, "COBS known-vector decode mismatch");
}

static void test_cobs_roundtrip_payloads(void)
{
    const uint8_t payload_a[] = {};
    const uint8_t payload_b[] = {0x41, 0x42, 0x43};
    const uint8_t payload_c[] = {0x00, 0x11, 0x00, 0x22, 0x00};
    const uint8_t payload_d[] = {0x11, 0x22, 0x33, 0x44, 0x00, 0x55, 0x66, 0x00, 0x77};

    const uint8_t *inputs[] = {payload_a, payload_b, payload_c, payload_d};
    const int input_sizes[] = {(int)sizeof(payload_a), (int)sizeof(payload_b), (int)sizeof(payload_c), (int)sizeof(payload_d)};

    uint8_t encoded[128] = {0};
    uint8_t decoded[128] = {0};

    for (int i = 0; i < 4; i++)
    {
        memset(encoded, 0, sizeof(encoded));
        memset(decoded, 0, sizeof(decoded));

        int encoded_len = datalink_cobs_encode(inputs[i], input_sizes[i], encoded, (int)sizeof(encoded));
        ASSERT_TRUE(encoded_len >= 1, "COBS encode failed");

        int decoded_len = datalink_cobs_decode(encoded, encoded_len, decoded, (int)sizeof(decoded));
        ASSERT_TRUE(decoded_len == input_sizes[i], "COBS decoded size mismatch");
        ASSERT_TRUE(memcmp(decoded, inputs[i], (size_t)input_sizes[i]) == 0, "COBS roundtrip payload mismatch");
    }
}

static void test_cobs_empty_input(void)
{
    const uint8_t payload[] = {};
    const uint8_t expected[] = {0x01};

    uint8_t encoded[16] = {0};
    int encoded_len = datalink_cobs_encode(payload, (int)sizeof(payload), encoded, (int)sizeof(encoded));

    ASSERT_TRUE(encoded_len == (int)sizeof(expected), "unexpected COBS encoded length for empty input");
    ASSERT_TRUE(memcmp(encoded, expected, sizeof(expected)) == 0, "COBS encode mismatch for empty input");
}

static void test_cobs_decode_empty_input(void)
{
    const uint8_t payload[] = {};
    uint8_t buffer[16] = {0};
    int len = datalink_cobs_decode(payload, (int)sizeof(payload), buffer, (int)sizeof(buffer));

    ASSERT_TRUE(len == 0, "unexpected COBS decoded length for empty input");
}

static void test_cobs_encode_buffer_overflow_protection(void)
{
    const uint8_t data[] = {0x11, 0x22, 0x00, 0x33};
    uint8_t buffer[12];
    const uint8_t canary = 0xCD;

    memset(buffer, canary, sizeof(buffer));

    ASSERT_TRUE(datalink_cobs_encode(data, (int)sizeof(data), buffer, 4) == -1, "expected COBS encode small-buffer failure");
    assert_canary_untouched(buffer, 4, (int)sizeof(buffer), canary, "COBS encode wrote past declared buffer_size");
}

static void test_cobs_decode_buffer_overflow_protection(void)
{
    const uint8_t encoded[] = {0x03, 0x11, 0x22, 0x02, 0x33};
    uint8_t decoded[12];
    const uint8_t canary = 0xA5;

    memset(decoded, canary, sizeof(decoded));

    ASSERT_TRUE(datalink_cobs_decode(encoded, (int)sizeof(encoded), decoded, 2) == -1, "expected COBS decode small-buffer failure");
    assert_canary_untouched(decoded, 2, (int)sizeof(decoded), canary, "COBS decode wrote past declared data_size");
}

static void test_message_payload_max_length_roundtrip(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[300] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 13;
    msg.len = DATALINK_MESSAGE_MAX_PAYLOAD_SIZE;
    fill_payload(&msg, 0x01);

    ASSERT_TRUE(datalink_serialize_message(&msg, buffer, &len) == DATALINK_OK, "max-payload serialize failed");
    ASSERT_TRUE(len == msg.len + 2, "max-payload serialized length mismatch");
    ASSERT_TRUE(datalink_deserialize_message(&parsed, buffer, len) == DATALINK_OK, "max-payload deserialize failed");
    ASSERT_TRUE(parsed.msg_id == msg.msg_id, "max-payload msg_id mismatch");
    ASSERT_TRUE(parsed.len == msg.len, "max-payload len mismatch");
    ASSERT_TRUE(memcmp(parsed.payload, msg.payload, msg.len) == 0, "max-payload data mismatch");
}

static void test_message_rejects_len_above_max_payload(void)
{
    datalink_message_t parsed = {0};
    uint8_t buffer[257] = {0};

    buffer[0] = 0xAA;
    buffer[1] = 0xFF;
    memset(buffer + 2, 0x55, 255);

    ASSERT_TRUE(datalink_deserialize_message(&parsed, buffer, (int)sizeof(buffer)) == DATALINK_ERROR, "expected >max payload rejection");
}

static void test_message_serialize_buffer_overflow_protection(void)
{
    datalink_message_t msg = {0};
    uint8_t buffer[16];
    int len = 7;
    const uint8_t canary = 0x5C;

    msg.msg_id = 3;
    msg.len = 6;
    fill_payload(&msg, 0x20);

    memset(buffer, canary, sizeof(buffer));

    ASSERT_TRUE(datalink_serialize_message(&msg, buffer, &len) == DATALINK_ERROR, "expected message serialize small-buffer failure");
    assert_canary_untouched(buffer, 7, (int)sizeof(buffer), canary, "message serialize wrote past declared len");
}

static void test_serial_serialize_buffer_overflow_protection(void)
{
    datalink_message_t msg = {0};
    uint8_t buffer[64];
    int len = 10;
    const uint8_t canary = 0x4A;

    msg.msg_id = 8;
    msg.len = 20;
    fill_payload(&msg, 0x31);

    memset(buffer, canary, sizeof(buffer));

    ASSERT_TRUE(datalink_serialize_message_serial(&msg, buffer, &len) == DATALINK_ERROR, "expected serial serialize small-buffer failure");
    assert_canary_untouched(buffer, 10, (int)sizeof(buffer), canary, "serial serialize wrote past declared len");
}

static void test_radio_serialize_buffer_overflow_protection(void)
{
    datalink_message_t msg = {0};
    uint8_t buffer[64];
    int len = 12;
    const uint8_t canary = 0x3E;

    msg.msg_id = 15;
    msg.len = 10;
    fill_payload(&msg, 0x41);

    memset(buffer, canary, sizeof(buffer));

    ASSERT_TRUE(datalink_serialize_message_radio(&msg, 1, 2, 3, buffer, &len) == DATALINK_ERROR, "expected radio serialize small-buffer failure");
    assert_canary_untouched(buffer, 12, (int)sizeof(buffer), canary, "radio serialize wrote past declared len");
}

static void test_message_roundtrip(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[300] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 7;
    msg.len = 16;
    fill_payload(&msg, 0x10);

    ASSERT_TRUE(datalink_serialize_message(&msg, buffer, &len) == DATALINK_OK, "serialize failed");
    ASSERT_TRUE(datalink_deserialize_message(&parsed, buffer, len) == DATALINK_OK, "deserialize failed");
    ASSERT_TRUE(parsed.msg_id == msg.msg_id, "msg_id mismatch");
    ASSERT_TRUE(parsed.len == msg.len, "len mismatch");
    ASSERT_TRUE(memcmp(parsed.payload, msg.payload, msg.len) == 0, "payload mismatch");
}

static void test_message_rejects_short_data(void)
{
    datalink_message_t parsed = {0};
    uint8_t buffer[1] = {0x01};

    ASSERT_TRUE(datalink_deserialize_message(&parsed, buffer, 1) == DATALINK_ERROR, "expected short-data error");
}

static void test_message_rejects_len_mismatch(void)
{
    datalink_message_t parsed = {0};
    uint8_t buffer[4] = {0x01, 0x03, 0xAA, 0xBB};

    ASSERT_TRUE(datalink_deserialize_message(&parsed, buffer, 4) == DATALINK_ERROR, "expected length-mismatch error");
}

static void test_message_serialize_buffer_too_small(void)
{
    datalink_message_t msg = {0};
    uint8_t buffer[4] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 3;
    msg.len = 8;
    fill_payload(&msg, 0x21);

    ASSERT_TRUE(datalink_serialize_message(&msg, buffer, &len) == DATALINK_ERROR, "expected serialize buffer-too-small error");
}

static void test_serial_roundtrip(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[400] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 4;
    msg.len = 10;
    fill_payload(&msg, 0x30);

    ASSERT_TRUE(datalink_serialize_message_serial(&msg, buffer, &len) == DATALINK_OK, "serial serialize failed");
    ASSERT_TRUE(datalink_deserialize_message_serial(&parsed, buffer, len) == DATALINK_OK, "serial deserialize failed");
    ASSERT_TRUE(parsed.msg_id == msg.msg_id, "serial msg_id mismatch");
    ASSERT_TRUE(parsed.len == msg.len, "serial len mismatch");
    ASSERT_TRUE(memcmp(parsed.payload, msg.payload, msg.len) == 0, "serial payload mismatch");
}

static void test_serial_all_zero_payload_roundtrip(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[300] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 5;
    msg.len = 64;
    memset(msg.payload, 0, msg.len);

    ASSERT_TRUE(datalink_serialize_message_serial(&msg, buffer, &len) == DATALINK_OK, "serial all-zero serialize failed");
    ASSERT_TRUE(datalink_deserialize_message_serial(&parsed, buffer, len) == DATALINK_OK, "serial all-zero deserialize failed");
    ASSERT_TRUE(parsed.msg_id == msg.msg_id, "serial all-zero msg_id mismatch");
    ASSERT_TRUE(parsed.len == msg.len, "serial all-zero len mismatch");
    ASSERT_TRUE(memcmp(parsed.payload, msg.payload, msg.len) == 0, "serial all-zero payload mismatch");
}

static void test_serial_rejects_tampered_frame(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[300] = {0};
    int len = (int)sizeof(buffer);

    msg.msg_id = 9;
    msg.len = 12;
    fill_payload(&msg, 0x40);

    ASSERT_TRUE(datalink_serialize_message_serial(&msg, buffer, &len) == DATALINK_OK, "serial serialize failed");
    ASSERT_TRUE(len > 3, "serialized frame too short for tamper");

    buffer[1] ^= 0x01;

    ASSERT_TRUE(datalink_deserialize_message_serial(&parsed, buffer, len) == DATALINK_ERROR, "expected tampered serial frame error");
}

static void test_serial_rejects_too_short(void)
{
    datalink_message_t parsed = {0};
    uint8_t frame[1] = {0x00};

    ASSERT_TRUE(datalink_deserialize_message_serial(&parsed, frame, 1) == DATALINK_ERROR, "expected short serial frame error");
}

static void test_radio_roundtrip(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[400] = {0};
    int len = (int)sizeof(buffer);
    uint8_t seq = 0;
    uint8_t src = 0;
    uint8_t dst = 0;

    msg.msg_id = 11;
    msg.len = 14;
    fill_payload(&msg, 0x50);

    ASSERT_TRUE(datalink_serialize_message_radio(&msg, 5, 7, 9, buffer, &len) == DATALINK_OK, "radio serialize failed");
    ASSERT_TRUE(datalink_deserialize_message_radio(&parsed, &seq, &src, &dst, buffer, len) == DATALINK_OK, "radio deserialize failed");
    ASSERT_TRUE(seq == 5 && src == 7 && dst == 9, "radio header mismatch");
    ASSERT_TRUE(parsed.msg_id == msg.msg_id, "radio msg_id mismatch");
    ASSERT_TRUE(parsed.len == msg.len, "radio len mismatch");
    ASSERT_TRUE(memcmp(parsed.payload, msg.payload, msg.len) == 0, "radio payload mismatch");
}

static void test_radio_rejects_tampered_frame(void)
{
    datalink_message_t msg = {0};
    datalink_message_t parsed = {0};
    uint8_t buffer[300] = {0};
    int len = (int)sizeof(buffer);
    uint8_t seq = 0;
    uint8_t src = 0;
    uint8_t dst = 0;

    msg.msg_id = 12;
    msg.len = 10;
    fill_payload(&msg, 0x60);

    ASSERT_TRUE(datalink_serialize_message_radio(&msg, 1, 2, 3, buffer, &len) == DATALINK_OK, "radio serialize failed");
    ASSERT_TRUE(len > 4, "serialized radio frame too short for tamper");

    buffer[3] ^= 0x01;

    ASSERT_TRUE(datalink_deserialize_message_radio(&parsed, &seq, &src, &dst, buffer, len) == DATALINK_ERROR, "expected tampered radio frame error");
}

static void test_radio_rejects_too_short(void)
{
    datalink_message_t parsed = {0};
    uint8_t seq = 0;
    uint8_t src = 0;
    uint8_t dst = 0;
    uint8_t frame[2] = {0x5A, 0x00};

    ASSERT_TRUE(datalink_deserialize_message_radio(&parsed, &seq, &src, &dst, frame, 2) == DATALINK_ERROR, "expected short radio frame error");
}

int main(void)
{
    RUN_TEST(test_crc16_known_vector);
    RUN_TEST(test_cobs_known_vector_encode_decode);
    RUN_TEST(test_cobs_roundtrip_payloads);
    RUN_TEST(test_cobs_empty_input);
    RUN_TEST(test_cobs_decode_empty_input);
    RUN_TEST(test_cobs_encode_buffer_overflow_protection);
    RUN_TEST(test_cobs_decode_buffer_overflow_protection);
    RUN_TEST(test_message_payload_max_length_roundtrip);
    RUN_TEST(test_message_rejects_len_above_max_payload);
    RUN_TEST(test_message_serialize_buffer_overflow_protection);
    RUN_TEST(test_serial_serialize_buffer_overflow_protection);
    RUN_TEST(test_radio_serialize_buffer_overflow_protection);
    RUN_TEST(test_message_roundtrip);
    RUN_TEST(test_message_rejects_short_data);
    RUN_TEST(test_message_rejects_len_mismatch);
    RUN_TEST(test_message_serialize_buffer_too_small);
    RUN_TEST(test_serial_roundtrip);
    RUN_TEST(test_serial_all_zero_payload_roundtrip);
    RUN_TEST(test_serial_rejects_tampered_frame);
    RUN_TEST(test_serial_rejects_too_short);
    RUN_TEST(test_radio_roundtrip);
    RUN_TEST(test_radio_rejects_tampered_frame);
    RUN_TEST(test_radio_rejects_too_short);

    printf("\nPassed: %d, Failed: %d\n", g_passed, g_failed);

    return g_failed == 0 ? 0 : 1;
}